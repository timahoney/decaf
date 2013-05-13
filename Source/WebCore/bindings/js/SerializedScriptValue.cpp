/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "config.h"
#include "SerializedScriptValue.h"

#include "Blob.h"
#include "DataView.h"
#include "ExceptionCode.h"
#include "File.h"
#include "FileList.h"
#include "ImageData.h"
#include "JSScriptState.h"
#include "JSScriptValue.h"
#include "JSSerializationDelegate.h"
#include "NotImplemented.h"
#include "RBArrayBuffer.h"
#include "RBConverters.h"
#include "RBDOMBinding.h"
#include "RBMessagePort.h"
#include "RBScriptState.h"
#include "RBScriptValue.h"
#include "RBSerializationDelegate.h"
#include "ScriptValue.h"
#include "SerializationDelegate.h"
#include "SerializationTags.h"
#include "SharedBuffer.h"
#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/APIShims.h>
#include <limits>
#include <wtf/ArrayBuffer.h>
#include <wtf/ArrayBufferView.h>
#include <wtf/HashTraits.h>
#include <wtf/Int8Array.h>
#include <wtf/Uint8Array.h>
#include <wtf/Uint8ClampedArray.h>
#include <wtf/Int16Array.h>
#include <wtf/Uint16Array.h>
#include <wtf/Int32Array.h>
#include <wtf/Uint32Array.h>
#include <wtf/Float32Array.h>
#include <wtf/Float64Array.h>
#include <wtf/Vector.h>

using namespace std;

#if CPU(BIG_ENDIAN) || CPU(MIDDLE_ENDIAN) || CPU(NEEDS_ALIGNED_ACCESS)
#define ASSUME_LITTLE_ENDIAN 0
#else
#define ASSUME_LITTLE_ENDIAN 1
#endif

namespace WebCore {

static const unsigned maximumFilterRecursion = 40000;

enum WalkerState { StateUnknown, ArrayStartState, ArrayStartVisitMember, ArrayEndVisitMember,
    ObjectStartState, ObjectStartVisitMember, ObjectEndVisitMember };

static unsigned typedArrayElementSize(ArrayBufferViewSubtag tag)
{
    switch (tag) {
    case DataViewTag:
    case Int8ArrayTag:
    case Uint8ArrayTag:
    case Uint8ClampedArrayTag:
        return 1;
    case Int16ArrayTag:
    case Uint16ArrayTag:
        return 2;
    case Int32ArrayTag:
    case Uint32ArrayTag:
    case Float32ArrayTag:
        return 4;
    case Float64ArrayTag:
        return 8;
    default:
        return 0;
    }

}

/* CurrentVersion tracks the serialization version so that persistant stores
 * are able to correctly bail out in the case of encountering newer formats.
 *
 * Initial version was 1.
 * Version 2. added the ObjectReferenceTag and support for serialization of cyclic graphs.
 * Version 3. added the FalseObjectTag, TrueObjectTag, NumberObjectTag, StringObjectTag
 * and EmptyStringObjectTag for serialization of Boolean, Number and String objects.
 * Version 4. added support for serializing non-index properties of arrays.
 */
static const unsigned CurrentVersion = 4;
static const unsigned TerminatorTag = 0xFFFFFFFF;
static const unsigned StringPoolTag = 0xFFFFFFFE;
static const unsigned NonIndexPropertiesTag = 0xFFFFFFFD;

/*
 * Object serialization is performed according to the following grammar, all tags
 * are recorded as a single uint8_t.
 *
 * IndexType (used for the object pool and StringData's constant pool) is the
 * minimum sized unsigned integer type required to represent the maximum index
 * in the constant pool.
 *
 * SerializedValue :- <CurrentVersion:uint32_t> Value
 * Value :- Array | Object | Terminal
 *
 * Array :-
 *     ArrayTag <length:uint32_t>(<index:uint32_t><value:Value>)* TerminatorTag
 *
 * Object :-
 *     ObjectTag (<name:StringData><value:Value>)* TerminatorTag
 *
 * Terminal :-
 *      UndefinedTag
 *    | NullTag
 *    | IntTag <value:int64_t>
 *    | ZeroTag
 *    | OneTag
 *    | FalseTag
 *    | TrueTag
 *    | FalseObjectTag
 *    | TrueObjectTag
 *    | DoubleTag <value:double>
 *    | NumberObjectTag <value:double>
 *    | DateTag <value:double>
 *    | String
 *    | EmptyStringTag
 *    | EmptyStringObjectTag
 *    | File
 *    | FileList
 *    | ImageData
 *    | Blob
 *    | ObjectReference
 *    | MessagePortReferenceTag <value:uint32_t>
 *    | ArrayBuffer
 *    | ArrayBufferViewTag ArrayBufferViewSubtag <byteOffset:uint32_t> <byteLenght:uint32_t> (ArrayBuffer | ObjectReference)
 *    | ArrayBufferTransferTag <value:uint32_t>
 *
 * String :-
 *      EmptyStringTag
 *      StringTag StringData
 *
 * StringObject:
 *      EmptyStringObjectTag
 *      StringObjectTag StringData
 *
 * StringData :-
 *      StringPoolTag <cpIndex:IndexType>
 *      (not (TerminatorTag | StringPoolTag))<length:uint32_t><characters:UChar{length}> // Added to constant pool when seen, string length 0xFFFFFFFF is disallowed
 *
 * File :-
 *    FileTag FileData
 *
 * FileData :-
 *    <path:StringData> <url:StringData> <type:StringData>
 *
 * FileList :-
 *    FileListTag <length:uint32_t>(<file:FileData>){length}
 *
 * ImageData :-
 *    ImageDataTag <width:int32_t><height:int32_t><length:uint32_t><data:uint8_t{length}>
 *
 * Blob :-
 *    BlobTag <url:StringData><type:StringData><size:long long>
 *
 * RegExp :-
 *    RegExpTag <pattern:StringData><flags:StringData>
 *
 * ObjectReference :-
 *    ObjectReferenceTag <opIndex:IndexType>
 *
 * ArrayBuffer :-
 *    ArrayBufferTag <length:uint32_t> <contents:byte{length}>
 */

typedef pair<ScriptValue, SerializationReturnCode> DeserializationResult;

class CloneBase {
protected:
    CloneBase(SerializationDelegate* delegate)
        : m_failed(false)
        , m_delegate(delegate)
    {
    }

    bool shouldTerminate()
    {
        return m_delegate->shouldTerminate();
    }

    unsigned ticksUntilNextCheck()
    {
        return m_delegate->ticksUntilNextCheck();
    }

    bool didTimeOut()
    {
        return m_delegate->didTimeOut();
    }

    void throwStackOverflow()
    {
        m_delegate->throwStackOverflow();
    }

    void throwInterruptedException()
    {
        m_delegate->throwInterruptedException();
    }

    NO_RETURN_DUE_TO_ASSERT
    void fail()
    {
        ASSERT_NOT_REACHED();
        m_failed = true;
    }

    bool m_failed;
    Vector<ScriptValue> m_gcBuffer;
    SerializationDelegate* m_delegate;
};

#if ASSUME_LITTLE_ENDIAN
template <typename T> static void writeLittleEndian(Vector<uint8_t>& buffer, T value)
{
    buffer.append(reinterpret_cast<uint8_t*>(&value), sizeof(value));
}
#else
template <typename T> static void writeLittleEndian(Vector<uint8_t>& buffer, T value)
{
    for (unsigned i = 0; i < sizeof(T); i++) {
        buffer.append(value & 0xFF);
        value >>= 8;
    }
}
#endif

template <> void writeLittleEndian<uint8_t>(Vector<uint8_t>& buffer, uint8_t value)
{
    buffer.append(value);
}

template <typename T> static bool writeLittleEndian(Vector<uint8_t>& buffer, const T* values, uint32_t length)
{
    if (length > numeric_limits<uint32_t>::max() / sizeof(T))
        return false;

#if ASSUME_LITTLE_ENDIAN
    buffer.append(reinterpret_cast<const uint8_t*>(values), length * sizeof(T));
#else
    for (unsigned i = 0; i < length; i++) {
        T value = values[i];
        for (unsigned j = 0; j < sizeof(T); j++) {
            buffer.append(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }
#endif
    return true;
}

class CloneSerializer : CloneBase {
public:
    static SerializationReturnCode serialize(SerializationDelegate* delegate, const ScriptValue& value,
                                             MessagePortArray* messagePorts, ArrayBufferArray* arrayBuffers,
                                             Vector<String>& blobURLs, Vector<uint8_t>& out)
    {
        CloneSerializer serializer(delegate, messagePorts, arrayBuffers, blobURLs, out);
        return serializer.serialize(value);
    }

    static bool serialize(const String& s, Vector<uint8_t>& out)
    {
        writeLittleEndian(out, CurrentVersion);
        if (s.isEmpty()) {
            writeLittleEndian<uint8_t>(out, EmptyStringTag);
            return true;
        }
        writeLittleEndian<uint8_t>(out, StringTag);
        writeLittleEndian(out, s.length());
        return writeLittleEndian(out, s.impl()->characters(), s.length());
    }

    static void serializeUndefined(Vector<uint8_t>& out)
    {
        writeLittleEndian(out, CurrentVersion);
        writeLittleEndian<uint8_t>(out, UndefinedTag);
    }

    static void serializeBoolean(bool value, Vector<uint8_t>& out)
    {
        writeLittleEndian(out, CurrentVersion);
        writeLittleEndian<uint8_t>(out, value ? TrueTag : FalseTag);
    }

    static void serializeNumber(double value, Vector<uint8_t>& out)
    {
        writeLittleEndian(out, CurrentVersion);
        writeLittleEndian<uint8_t>(out, DoubleTag);
        union {
            double d;
            int64_t i;
        } u;
        u.d = value;
        writeLittleEndian(out, u.i);
    }

private:
    typedef Vector<ScriptValue> ObjectPool;

    CloneSerializer(SerializationDelegate* delegate, MessagePortArray* messagePorts, ArrayBufferArray* arrayBuffers, Vector<String>& blobURLs, Vector<uint8_t>& out)
        : CloneBase(delegate)
        , m_buffer(out)
        , m_blobURLs(blobURLs)
    {
        write(CurrentVersion);
        fillTransferMap(messagePorts, m_transferredMessagePorts);
        fillTransferMap(arrayBuffers, m_transferredArrayBuffers);
    }

    template <class T>
    void fillTransferMap(Vector<RefPtr<T>, 1>* input, ObjectPool& result)
    {
        if (!input)
            return;
        for (size_t i = 0; i < input->size(); i++) {
            ScriptValue obj = m_delegate->toScriptValue(input->at(i).get());
            if (!obj.hasNoValue() && !result.contains(obj))
                result.append(obj);
        }
    }

    SerializationReturnCode serialize(const ScriptValue& in);

    bool isArray(const ScriptValue& value)
    {
        return m_delegate->isArray(value);
    }

    bool checkForDuplicate(const ScriptValue& object)
    {
        // Record object for graph reconstruction
        size_t index = m_objectPool.find(object);

        // Handle duplicate references
        if (index != notFound) {
            write(ObjectReferenceTag);
            ASSERT(index < m_objectPool.size());
            writeObjectIndex(index);
            return true;
        }

        return false;
    }

    void recordObject(const ScriptValue& object)
    {
        m_objectPool.append(object);
        m_gcBuffer.append(object);
    }

    bool startObjectInternal(const ScriptValue& object)
    {
        if (checkForDuplicate(object))
            return false;
        recordObject(object);
        return true;
    }

    bool startObject(const ScriptValue& object)
    {
        if (!startObjectInternal(object))
            return false;
        write(ObjectTag);
        return true;
    }

    bool startArray(const ScriptValue& array)
    {
        if (!startObjectInternal(array))
            return false;

        unsigned length = m_delegate->arrayLength(array);
        write(ArrayTag);
        write(length);
        return true;
    }

    void endObject()
    {
        write(TerminatorTag);
    }

    ScriptValue getProperty(const ScriptValue& value, const char* propertyName)
    {
        return m_delegate->getProperty(value, propertyName);
    }

    void dumpImmediate(const ScriptValue& value)
    {
        if (value.isNull())
            write(NullTag);
        else if (value.isUndefined())
            write(UndefinedTag);
        else if (value.isNumber()) {
            if (value.isInt64()) {
                if (!value.asInt64())
                    write(ZeroTag);
                else if (value.asInt64() == 1)
                    write(OneTag);
                else {
                    write(IntTag);
                    write(value.asInt64());
                }
            } else {
                write(DoubleTag);
                write(value.asDouble());
            }
        } else if (value.isBoolean()) {
            if (value.isTrue())
                write(TrueTag);
            else
                write(FalseTag);
        }
    }

    void dumpString(String str)
    {
        if (str.isEmpty())
            write(EmptyStringTag);
        else {
            write(StringTag);
            write(str);
        }
    }

    void dumpStringObject(String str)
    {
        if (str.isEmpty())
            write(EmptyStringObjectTag);
        else {
            write(StringObjectTag);
            write(str);
        }
    }

    bool dumpArrayBufferView(const ScriptValue& scriptValue, SerializationReturnCode& code)
    {
        write(ArrayBufferViewTag);

        RefPtr<ArrayBufferView> arrayBufferView = m_delegate->toArrayBufferView(scriptValue);

        switch (arrayBufferView->getType()) {
        case ArrayBufferView::TypeDataView:
            write(DataViewTag);
            break;
        case ArrayBufferView::TypeUint8Clamped:
            write(Uint8ClampedArrayTag);
            break;
        case ArrayBufferView::TypeInt8:
            write(Int8ArrayTag);
            break;
        case ArrayBufferView::TypeUint8:
            write(Uint8ArrayTag);
            break;
        case ArrayBufferView::TypeInt16:
            write(Int16ArrayTag);
            break;
        case ArrayBufferView::TypeUint16:
            write(Uint16ArrayTag);
            break;
        case ArrayBufferView::TypeInt32:
            write(Int32ArrayTag);
            break;
        case ArrayBufferView::TypeUint32:
            write(Uint32ArrayTag);
            break;
        case ArrayBufferView::TypeFloat32:
            write(Float32ArrayTag);
            break;
        case ArrayBufferView::TypeFloat64:
            write(Float64ArrayTag);
            break;
        default:
            return false;
        }    

        write(static_cast<uint32_t>(arrayBufferView->byteOffset()));
        write(static_cast<uint32_t>(arrayBufferView->byteLength()));
        RefPtr<ArrayBuffer> arrayBuffer = arrayBufferView->buffer();
        if (!arrayBuffer) {
            code = ValidationError;
            return true;
        }

        ScriptValue bufferObj = m_delegate->toScriptValue(arrayBuffer.get());
        return dumpIfTerminal(bufferObj, code);
    }

    bool dumpIfTerminal(const ScriptValue& value, SerializationReturnCode& code)
    {
        if (!value.isCell()) {
            dumpImmediate(value);
            return true;
        }

        if (value.isString()) {
            String str = m_delegate->asString(value);
            dumpString(str);
            return true;
        }

        if (value.isNumber()) {
            write(DoubleTag);
            write(value.asNumber());
            return true;
        }

        if (value.isObject() && m_delegate->isDate(value)) {
            write(DateTag);
            write(m_delegate->asDateValue(value));
            return true;
        }

        if (isArray(value))
            return false;

        if (value.isObject()) {
            SerializationTag tag = m_delegate->tagForObject(value);
            switch (tag) {
            case TrueObjectTag:
            case FalseObjectTag:
                if (!startObjectInternal(value)) // handle duplicates
                    return true;
                write(tag);
                return true;
                
            case StringObjectTag: {
                if (!startObjectInternal(value)) // handle duplicates
                    return true;
                String str = m_delegate->asString(value);
                dumpStringObject(str);
                return true;
            }
            case NumberObjectTag: {
                if (!startObjectInternal(value)) // handle duplicates
                    return true;
                write(NumberObjectTag);
                write(m_delegate->asNumber(value));
                return true;
            }
            case FileTag: {
                write(FileTag);
                write(m_delegate->toFile(value));
                return true;
            }
            case FileListTag: {
                FileList* list = m_delegate->toFileList(value);
                write(FileListTag);
                unsigned length = list->length();
                write(length);
                for (unsigned i = 0; i < length; i++)
                    write(list->item(i));
                return true;
            }
            case BlobTag: {
                write(BlobTag);
                Blob* blob = m_delegate->toBlob(value);
                m_blobURLs.append(blob->url());
                write(blob->url());
                write(blob->type());
                write(blob->size());
                return true;
            }
            case ImageDataTag: {
                ImageData* data = m_delegate->toImageData(value);
                write(ImageDataTag);
                write(data->width());
                write(data->height());
                write(data->data()->length());
                write(data->data()->data(), data->data()->length());
                return true;
            }
            case RegExpTag: {
                char flags[3];
                int flagCount = 0;
                if (m_delegate->isRegExpGlobal(value))
                    flags[flagCount++] = 'g';
                if (m_delegate->isRegExpIgnoreCase(value))
                    flags[flagCount++] = 'i';
                if (m_delegate->isRegExpMultiline(value))
                    flags[flagCount++] = 'm';
                write(RegExpTag);
                write(m_delegate->asRegExpPattern(value));
                write(String(flags, flagCount));
                return true;
            }
            case MessagePortReferenceTag: {
                size_t index = m_transferredMessagePorts.find(value);
                if (index != notFound) {
                    write(MessagePortReferenceTag);
                    write(static_cast<uint32_t>(index));
                    return true;
                }
                // MessagePort object could not be found in transferred message ports
                code = ValidationError;
                return true;
            }
            case ArrayBufferTag: {
                RefPtr<ArrayBuffer> arrayBuffer = m_delegate->toArrayBuffer(value);
                if (arrayBuffer->isNeutered()) {
                    code = ValidationError;
                    return true;
                }
                size_t index = m_transferredArrayBuffers.find(value);
                if (index != notFound) {
                    write(ArrayBufferTransferTag);
                    write(static_cast<uint32_t>(index));
                    return true;
                }
                if (!startObjectInternal(value)) // handle duplicates
                    return true;
                write(ArrayBufferTag);
                write(arrayBuffer->byteLength());
                write(static_cast<const uint8_t *>(arrayBuffer->data()), arrayBuffer->byteLength());
                return true;
            }
            case ArrayBufferViewTag: {
                if (checkForDuplicate(value))
                    return true;
                bool success = dumpArrayBufferView(value, code);
                recordObject(value);
                return success;
            }
                    
            default:
                return false;
            }
        }
        // Any other types are expected to serialize as null.
        write(NullTag);
        return true;
    }

    void write(SerializationTag tag)
    {
        writeLittleEndian<uint8_t>(m_buffer, static_cast<uint8_t>(tag));
    }

    void write(ArrayBufferViewSubtag tag)
    {
        writeLittleEndian<uint8_t>(m_buffer, static_cast<uint8_t>(tag));
    }

    void write(uint8_t c)
    {
        writeLittleEndian(m_buffer, c);
    }

    void write(uint32_t i)
    {
        writeLittleEndian(m_buffer, i);
    }
    
    void write(int64_t i)
    {
        writeLittleEndian(m_buffer, i);
    }

    void write(double d)
    {
        union {
            double d;
            int64_t i;
        } u;
        u.d = d;
        writeLittleEndian(m_buffer, u.i);
    }

    void write(int32_t i)
    {
        writeLittleEndian(m_buffer, i);
    }

    void write(unsigned long long i)
    {
        writeLittleEndian(m_buffer, i);
    }
    
    void write(uint16_t ch)
    {
        writeLittleEndian(m_buffer, ch);
    }

    void writeStringIndex(unsigned i)
    {
        writeConstantPoolIndex(m_constantPool, i);
    }
    
    void writeObjectIndex(unsigned i)
    {
        writeConstantPoolIndex(m_objectPool, i);
    }

    template <class T> void writeConstantPoolIndex(const T& constantPool, unsigned i)
    {
        ASSERT(static_cast<int32_t>(i) < static_cast<int32_t>(constantPool.size()));
        if (constantPool.size() <= 0xFF)
            write(static_cast<uint8_t>(i));
        else if (constantPool.size() <= 0xFFFF)
            write(static_cast<uint16_t>(i));
        else
            write(static_cast<uint32_t>(i));
    }

    void write(const String& str)
    {
        if (str.isNull()) {
            write(emptyString());
        } else {
            StringConstantPool::AddResult addResult = m_constantPool.add(str.impl(), m_constantPool.size());
            if (!addResult.isNewEntry) {
                write(StringPoolTag);
                writeStringIndex(addResult.iterator->value);
                return;
            }

            // This condition is unlikely to happen as they would imply an ~8gb
            // string but we should guard against it anyway
            if (str.length() >= StringPoolTag) {
                fail();
                return;
            }

            // Guard against overflow
            if (str.length() > (numeric_limits<uint32_t>::max() - sizeof(uint32_t)) / sizeof(UChar)) {
                fail();
                return;
            }

            writeLittleEndian<uint32_t>(m_buffer, str.length());
            if (!writeLittleEndian<uint16_t>(m_buffer, reinterpret_cast<const uint16_t*>(str.characters()), str.length()))
                fail();
        }
    }

    void write(const File* file)
    {
        m_blobURLs.append(file->url());
        write(file->path());
        write(file->url());
        write(file->type());
    }

    void write(const uint8_t* data, unsigned length)
    {
        m_buffer.append(data, length);
    }

    Vector<uint8_t>& m_buffer;
    Vector<String>& m_blobURLs;
    ObjectPool m_objectPool;
    ObjectPool m_transferredMessagePorts;
    ObjectPool m_transferredArrayBuffers;
    typedef HashMap<RefPtr<StringImpl>, uint32_t> StringConstantPool;
    StringConstantPool m_constantPool;
};

SerializationReturnCode CloneSerializer::serialize(const ScriptValue& in)
{
    Vector<uint32_t, 16> indexStack;
    Vector<uint32_t, 16> lengthStack;
    Vector<Vector<String>, 16> propertyStack;
    Vector<ScriptValue, 32> inputObjectStack;
    Vector<WalkerState, 16> stateStack;
    WalkerState state = StateUnknown;
    ScriptValue inValue = in;
    unsigned tickCount = ticksUntilNextCheck();
    while (1) {
        switch (state) {
            arrayStartState:
            case ArrayStartState: {
                ASSERT(m_delegate->isArray(inValue));
                if (inputObjectStack.size() > maximumFilterRecursion)
                    return StackOverflowError;

                unsigned length = m_delegate->arrayLength(inValue);
                if (!startArray(inValue))
                    break;
                inputObjectStack.append(inValue);
                indexStack.append(0);
                lengthStack.append(length);
                // fallthrough
            }
            arrayStartVisitMember:
            case ArrayStartVisitMember: {
                if (!--tickCount) {
                    if (didTimeOut())
                        return InterruptedExecutionError;
                    tickCount = ticksUntilNextCheck();
                }

                const ScriptValue& array = inputObjectStack.last();
                uint32_t index = indexStack.last();
                if (index == lengthStack.last()) {
                    indexStack.removeLast();
                    lengthStack.removeLast();

                    propertyStack.append(Vector<String>());
                    m_delegate->getOwnNonIndexPropertyNames(array, propertyStack.last());
                    if (propertyStack.last().size()) {
                        write(NonIndexPropertiesTag);
                        indexStack.append(0);
                        goto objectStartVisitMember;
                    }
                    propertyStack.removeLast();

                    endObject();
                    inputObjectStack.removeLast();
                    break;
                }
                inValue = m_delegate->objectAtIndex(array, index);
                if (inValue.hasNoValue()) {
                    indexStack.last()++;
                    goto arrayStartVisitMember;
                }

                write(index);
                SerializationReturnCode terminalCode = SuccessfullyCompleted;
                if (dumpIfTerminal(inValue, terminalCode)) {
                    if (terminalCode != SuccessfullyCompleted)
                        return terminalCode;
                    indexStack.last()++;
                    goto arrayStartVisitMember;
                }
                stateStack.append(ArrayEndVisitMember);
                goto stateUnknown;
            }
            case ArrayEndVisitMember: {
                indexStack.last()++;
                goto arrayStartVisitMember;
            }
            objectStartState:
            case ObjectStartState: {
                ASSERT(inValue.isObject());
                if (inputObjectStack.size() > maximumFilterRecursion)
                    return StackOverflowError;
                if (!startObject(inValue))
                    break;
                // At this point, all supported objects other than Object
                // objects have been handled. If we reach this point and
                // the input is not an Object object then we should throw
                // a DataCloneError.
                if (!m_delegate->isFinalObject(inValue))
                    return DataCloneError;
                inputObjectStack.append(inValue);
                indexStack.append(0);
                propertyStack.append(Vector<String>());
                m_delegate->getOwnPropertyNames(inValue, propertyStack.last());
                // fallthrough
            }
            objectStartVisitMember:
            case ObjectStartVisitMember: {
                if (!--tickCount) {
                    if (didTimeOut())
                        return InterruptedExecutionError;
                    tickCount = ticksUntilNextCheck();
                }

                ScriptValue& object = inputObjectStack.last();
                uint32_t index = indexStack.last();
                Vector<String>& properties = propertyStack.last();
                if (index == properties.size()) {
                    endObject();
                    inputObjectStack.removeLast();
                    indexStack.removeLast();
                    propertyStack.removeLast();
                    break;
                }
                inValue = m_delegate->getProperty(object, properties[index]);
                if (shouldTerminate())
                    return ExistingExceptionError;

                if (inValue.hasNoValue()) {
                    // Property was removed during serialisation
                    indexStack.last()++;
                    goto objectStartVisitMember;
                }
                write(properties[index]);

                if (shouldTerminate())
                    return ExistingExceptionError;

                SerializationReturnCode terminalCode = SuccessfullyCompleted;
                if (!dumpIfTerminal(inValue, terminalCode)) {
                    stateStack.append(ObjectEndVisitMember);
                    goto stateUnknown;
                }
                if (terminalCode != SuccessfullyCompleted)
                    return terminalCode;
                // fallthrough
            }
            case ObjectEndVisitMember: {
                if (shouldTerminate())
                    return ExistingExceptionError;

                indexStack.last()++;
                goto objectStartVisitMember;
            }
            stateUnknown:
            case StateUnknown: {
                SerializationReturnCode terminalCode = SuccessfullyCompleted;
                if (dumpIfTerminal(inValue, terminalCode)) {
                    if (terminalCode != SuccessfullyCompleted)
                        return terminalCode;
                    break;
                }

                if (m_delegate->isArray(inValue))
                    goto arrayStartState;
                goto objectStartState;
            }
        }
        if (stateStack.isEmpty())
            break;

        state = stateStack.last();
        stateStack.removeLast();

        if (!--tickCount) {
            if (didTimeOut())
                return InterruptedExecutionError;
            tickCount = ticksUntilNextCheck();
        }
    }
    if (m_failed)
        return UnspecifiedError;

    return SuccessfullyCompleted;
}

typedef Vector<WTF::ArrayBufferContents> ArrayBufferContentsArray;

class CloneDeserializer : CloneBase {
public:
    static String deserializeString(const Vector<uint8_t>& buffer)
    {
        const uint8_t* ptr = buffer.begin();
        const uint8_t* end = buffer.end();
        uint32_t version;
        if (!readLittleEndian(ptr, end, version) || version > CurrentVersion)
            return String();
        uint8_t tag;
        if (!readLittleEndian(ptr, end, tag) || tag != StringTag)
            return String();
        uint32_t length;
        if (!readLittleEndian(ptr, end, length) || length >= StringPoolTag)
            return String();
        String str;
        if (!readString(ptr, end, str, length))
            return String();
        return String(str.impl());
    }

    static DeserializationResult deserialize(SerializationDelegate* delegate,
                                             MessagePortArray* messagePorts, ArrayBufferContentsArray* arrayBufferContentsArray,
                                             const Vector<uint8_t>& buffer)
    {
        if (!buffer.size())
            return make_pair(delegate->null(), UnspecifiedError);
        CloneDeserializer deserializer(delegate, messagePorts, arrayBufferContentsArray, buffer);
        if (!deserializer.isValid())
            return make_pair(ScriptValue(), ValidationError);
        return deserializer.deserialize();
    }

private:
    struct CachedString {
        CachedString(const String& string)
            : m_string(string)
        {
        }

        ScriptValue& scriptString(SerializationDelegate* delegate)
        {
            if (m_scriptString.hasNoValue())
                m_scriptString = delegate->toScriptValue(m_string);
            return m_scriptString;
        }
        const String& string() { return m_string; }

    private:
        String m_string;
        ScriptValue m_scriptString;
    };

    struct CachedStringRef {
        CachedStringRef()
            : m_base(0)
            , m_index(0)
        {
        }
        CachedStringRef(Vector<CachedString>* base, size_t index)
            : m_base(base)
            , m_index(index)
        {
        }
        
        CachedString* operator->() { ASSERT(m_base); return &m_base->at(m_index); }
        
    private:
        Vector<CachedString>* m_base;
        size_t m_index;
    };

    CloneDeserializer(SerializationDelegate* delegate,
                      MessagePortArray* messagePorts, ArrayBufferContentsArray* arrayBufferContents,
                      const Vector<uint8_t>& buffer)
        : CloneBase(delegate)
        , m_ptr(buffer.data())
        , m_end(buffer.data() + buffer.size())
        , m_version(0xFFFFFFFF)
        , m_messagePorts(messagePorts)
        , m_arrayBufferContents(arrayBufferContents)
        , m_arrayBuffers(arrayBufferContents ? arrayBufferContents->size() : 0)
    {
        if (!read(m_version))
            m_version = 0xFFFFFFFF;
    }

    DeserializationResult deserialize();

    void throwValidationError()
    {
        m_delegate->throwValidationError();
    }

    bool isValid() const { return m_version <= CurrentVersion; }

    template <typename T> bool readLittleEndian(T& value)
    {
        if (m_failed || !readLittleEndian(m_ptr, m_end, value)) {
            fail();
            return false;
        }
        return true;
    }
#if ASSUME_LITTLE_ENDIAN
    template <typename T> static bool readLittleEndian(const uint8_t*& ptr, const uint8_t* end, T& value)
    {
        if (ptr > end - sizeof(value))
            return false;

        if (sizeof(T) == 1)
            value = *ptr++;
        else {
            value = *reinterpret_cast<const T*>(ptr);
            ptr += sizeof(T);
        }
        return true;
    }
#else
    template <typename T> static bool readLittleEndian(const uint8_t*& ptr, const uint8_t* end, T& value)
    {
        if (ptr > end - sizeof(value))
            return false;

        if (sizeof(T) == 1)
            value = *ptr++;
        else {
            value = 0;
            for (unsigned i = 0; i < sizeof(T); i++)
                value += ((T)*ptr++) << (i * 8);
        }
        return true;
    }
#endif

    bool read(uint32_t& i)
    {
        return readLittleEndian(i);
    }

    bool read(int32_t& i)
    {
        return readLittleEndian(*reinterpret_cast<uint32_t*>(&i));
    }
    
    bool read(int64_t& i)
    {
        return readLittleEndian(i);
    }

    bool read(uint16_t& i)
    {
        return readLittleEndian(i);
    }

    bool read(uint8_t& i)
    {
        return readLittleEndian(i);
    }

    bool read(double& d)
    {
        union {
            double d;
            uint64_t i64;
        } u;
        if (!readLittleEndian(u.i64))
            return false;
        d = u.d;
        return true;
    }

    bool read(unsigned long long& i)
    {
        return readLittleEndian(i);
    }

    bool readStringIndex(uint32_t& i)
    {
        return readConstantPoolIndex(m_constantPool, i);
    }

    template <class T> bool readConstantPoolIndex(const T& constantPool, uint32_t& i)
    {
        if (constantPool.size() <= 0xFF) {
            uint8_t i8;
            if (!read(i8))
                return false;
            i = i8;
            return true;
        }
        if (constantPool.size() <= 0xFFFF) {
            uint16_t i16;
            if (!read(i16))
                return false;
            i = i16;
            return true;
        }
        return read(i);
    }

    static bool readString(const uint8_t*& ptr, const uint8_t* end, String& str, unsigned length)
    {
        if (length >= numeric_limits<int32_t>::max() / sizeof(UChar))
            return false;

        unsigned size = length * sizeof(UChar);
        if ((end - ptr) < static_cast<int>(size))
            return false;

#if ASSUME_LITTLE_ENDIAN
        str = String(reinterpret_cast<const UChar*>(ptr), length);
        ptr += length * sizeof(UChar);
#else
        Vector<UChar> buffer;
        buffer.reserveCapacity(length);
        for (unsigned i = 0; i < length; i++) {
            uint16_t ch;
            readLittleEndian(ptr, end, ch);
            buffer.append(ch);
        }
        str = String::adopt(buffer);
#endif
        return true;
    }

    bool readStringData(CachedStringRef& cachedString)
    {
        bool scratch;
        return readStringData(cachedString, scratch);
    }

    bool readStringData(CachedStringRef& cachedString, bool& wasTerminator)
    {
        if (m_failed)
            return false;
        uint32_t length = 0;
        if (!read(length))
            return false;
        if (length == TerminatorTag) {
            wasTerminator = true;
            return false;
        }
        if (length == StringPoolTag) {
            unsigned index = 0;
            if (!readStringIndex(index)) {
                fail();
                return false;
            }
            if (index >= m_constantPool.size()) {
                fail();
                return false;
            }
            cachedString = CachedStringRef(&m_constantPool, index);
            return true;
        }
        String str;
        if (!readString(m_ptr, m_end, str, length)) {
            fail();
            return false;
        }
        m_constantPool.append(str);
        cachedString = CachedStringRef(&m_constantPool, m_constantPool.size() - 1);
        return true;
    }

    SerializationTag readTag()
    {
        if (m_ptr >= m_end)
            return ErrorTag;
        return static_cast<SerializationTag>(*m_ptr++);
    }

    bool readArrayBufferViewSubtag(ArrayBufferViewSubtag& tag)
    {
        if (m_ptr >= m_end)
            return false;
        tag = static_cast<ArrayBufferViewSubtag>(*m_ptr++);
        return true;
    }

    void putProperty(const ScriptValue& object, unsigned index, const ScriptValue& value)
    {
        m_delegate->setArrayElement(object, index, value);
    }

    void putProperty(const ScriptValue& object, const String& propertyName, const ScriptValue& value)
    {
        m_delegate->setObjectProperty(object, propertyName, value);
    }

    bool readFile(RefPtr<File>& file)
    {
        CachedStringRef path;
        if (!readStringData(path))
            return 0;
        CachedStringRef url;
        if (!readStringData(url))
            return 0;
        CachedStringRef type;
        if (!readStringData(type))
            return 0;
        file = File::create(path->string(), KURL(KURL(), url->string()), type->string());
        return true;
    }

    bool readArrayBuffer(RefPtr<ArrayBuffer>& arrayBuffer)
    {
        uint32_t length;
        if (!read(length))
            return false;
        if (m_ptr + length > m_end)
            return false;
        arrayBuffer = ArrayBuffer::create(m_ptr, length);
        m_ptr += length;
        return true;
    }

    bool readArrayBufferView(ScriptValue& arrayBufferView)
    {
        ArrayBufferViewSubtag arrayBufferViewSubtag;
        if (!readArrayBufferViewSubtag(arrayBufferViewSubtag))
            return false;
        uint32_t byteOffset;
        if (!read(byteOffset))
            return false;
        uint32_t byteLength;
        if (!read(byteLength))
            return false;
        ScriptValue arrayBufferObj = readTerminal();
        if (arrayBufferObj.hasNoValue() || m_delegate->tagForObject(arrayBufferObj) != ArrayBufferTag)
            return false;

        unsigned elementSize = typedArrayElementSize(arrayBufferViewSubtag);
        if (!elementSize)
            return false;
        unsigned length = byteLength / elementSize;
        if (length * elementSize != byteLength)
            return false;

        RefPtr<ArrayBuffer> arrayBuffer = m_delegate->toArrayBuffer(arrayBufferObj);
        switch (arrayBufferViewSubtag) {
        case DataViewTag:
            arrayBufferView = m_delegate->toScriptValue(DataView::create(arrayBuffer, byteOffset, length).get());
            return true;
        case Int8ArrayTag:
            arrayBufferView = m_delegate->toScriptValue(Int8Array::create(arrayBuffer, byteOffset, length).get());
            return true;
        case Uint8ArrayTag:
            arrayBufferView = m_delegate->toScriptValue(Uint8Array::create(arrayBuffer, byteOffset, length).get());
            return true;
        case Uint8ClampedArrayTag:
            arrayBufferView = m_delegate->toScriptValue(Uint8ClampedArray::create(arrayBuffer, byteOffset, length).get());
            return true;
        case Int16ArrayTag:
            arrayBufferView = m_delegate->toScriptValue(Int16Array::create(arrayBuffer, byteOffset, length).get());
            return true;
        case Uint16ArrayTag:
            arrayBufferView = m_delegate->toScriptValue(Uint16Array::create(arrayBuffer, byteOffset, length).get());
            return true;
        case Int32ArrayTag:
            arrayBufferView = m_delegate->toScriptValue(Int32Array::create(arrayBuffer, byteOffset, length).get());
            return true;
        case Uint32ArrayTag:
            arrayBufferView = m_delegate->toScriptValue(Uint32Array::create(arrayBuffer, byteOffset, length).get());
            return true;
        case Float32ArrayTag:
            arrayBufferView = m_delegate->toScriptValue(Float32Array::create(arrayBuffer, byteOffset, length).get());
            return true;
        case Float64ArrayTag:
            arrayBufferView = m_delegate->toScriptValue(Float64Array::create(arrayBuffer, byteOffset, length).get());
            return true;
        default:
            return false;
        }
    }

    ScriptValue readTerminal()
    {
        SerializationTag tag = readTag();
        switch (tag) {
        case UndefinedTag:
            return m_delegate->undefined();
        case NullTag:
            return m_delegate->null();
        case IntTag: {
            int64_t i;
            if (!read(i))
                return ScriptValue();
            return m_delegate->toScriptValue(i);
        }
        case ZeroTag:
            return m_delegate->toScriptValue(static_cast<int64_t>(0));
        case OneTag:
            return m_delegate->toScriptValue(static_cast<int64_t>(1));
        case FalseTag:
            return m_delegate->toScriptValue(false);
        case TrueTag:
            return m_delegate->toScriptValue(true);
        case FalseObjectTag: {
            ScriptValue value = m_delegate->toBooleanObjectScriptValue(false);
            m_gcBuffer.append(value);
            return value;
        }
        case TrueObjectTag: {
            ScriptValue value = m_delegate->toBooleanObjectScriptValue(true);
            m_gcBuffer.append(value);
            return value;
        }
        case DoubleTag: {
            double d;
            if (!read(d))
                return ScriptValue();
            return m_delegate->toScriptValue(d);
        }
        case NumberObjectTag: {
            double d;
            if (!read(d))
                return ScriptValue();
            ScriptValue value = m_delegate->toNumberObjectScriptValue(d);
            m_gcBuffer.append(value);
            return value;
        }
        case DateTag: {
            double d;
            if (!read(d))
                return ScriptValue();
            return m_delegate->toDateScriptValue(d);
        }
        case FileTag: {
            RefPtr<File> file;
            if (!readFile(file))
                return ScriptValue();
            return m_delegate->toScriptValue(file.get());
        }
        case FileListTag: {
            unsigned length = 0;
            if (!read(length))
                return ScriptValue();
            RefPtr<FileList> result = FileList::create();
            for (unsigned i = 0; i < length; i++) {
                RefPtr<File> file;
                if (!readFile(file))
                    return ScriptValue();
                result->append(file.get());
            }
            return m_delegate->toScriptValue(result.get());
        }
        case ImageDataTag: {
            int32_t width;
            if (!read(width))
                return ScriptValue();
            int32_t height;
            if (!read(height))
                return ScriptValue();
            uint32_t length;
            if (!read(length))
                return ScriptValue();
            if (m_end < ((uint8_t*)0) + length || m_ptr > m_end - length) {
                fail();
                return ScriptValue();
            }
            RefPtr<ImageData> result = ImageData::create(IntSize(width, height));
            memcpy(result->data()->data(), m_ptr, length);
            m_ptr += length;
            return m_delegate->toScriptValue(result.get());
        }
        case BlobTag: {
            CachedStringRef url;
            if (!readStringData(url))
                return ScriptValue();
            CachedStringRef type;
            if (!readStringData(type))
                return ScriptValue();
            unsigned long long size = 0;
            if (!read(size))
                return ScriptValue();
            
            RefPtr<Blob> result = Blob::create(KURL(KURL(), url->string()), type->string(), size);
            return m_delegate->toScriptValue(result.get());
        }
        case StringTag: {
            CachedStringRef cachedString;
            if (!readStringData(cachedString))
                return ScriptValue();
            return cachedString->scriptString(m_delegate);
        }
        case EmptyStringTag:
            return m_delegate->emptyString();
        case StringObjectTag: {
            CachedStringRef cachedString;
            if (!readStringData(cachedString))
                return ScriptValue();
            ScriptValue value = m_delegate->toStringObjectScriptValue(cachedString->string());
            m_gcBuffer.append(value);
            return value;
        }
        case EmptyStringObjectTag: {
            ScriptValue value = m_delegate->toStringObjectScriptValue(emptyString());
            m_gcBuffer.append(value);
            return value;
        }
        case RegExpTag: {
            CachedStringRef pattern;
            if (!readStringData(pattern))
                return ScriptValue();
            CachedStringRef flags;
            if (!readStringData(flags))
                return ScriptValue();
            return m_delegate->toRegExpScriptValue(flags->string(), pattern->string());
        }
        case ObjectReferenceTag: {
            unsigned index = 0;
            if (!readConstantPoolIndex(m_gcBuffer, index)) {
                fail();
                return ScriptValue();
            }
            return m_gcBuffer.at(index);
        }
        case MessagePortReferenceTag: {
            uint32_t index;
            bool indexSuccessfullyRead = read(index);
            if (!indexSuccessfullyRead || !m_messagePorts || index >= m_messagePorts->size()) {
                fail();
                return ScriptValue();
            }
            return m_delegate->toScriptValue(m_messagePorts->at(index).get());
        }
        case ArrayBufferTag: {
            RefPtr<ArrayBuffer> arrayBuffer;
            if (!readArrayBuffer(arrayBuffer)) {
                fail();
                return ScriptValue();
            }
            ScriptValue result = m_delegate->toScriptValue(arrayBuffer.get());
            m_gcBuffer.append(result);
            return result;
        }
        case ArrayBufferTransferTag: {
            uint32_t index;
            bool indexSuccessfullyRead = read(index);
            if (!indexSuccessfullyRead || index >= m_arrayBuffers.size()) {
                fail();
                return ScriptValue();
            }

            if (!m_arrayBuffers[index])
                m_arrayBuffers[index] = ArrayBuffer::create(m_arrayBufferContents->at(index));

            return m_delegate->toScriptValue(m_arrayBuffers[index].get());
        }
        case ArrayBufferViewTag: {
            ScriptValue arrayBufferView;
            if (!readArrayBufferView(arrayBufferView)) {
                fail();
                return ScriptValue();
            }
            m_gcBuffer.append(arrayBufferView);
            return arrayBufferView;
        }
        default:
            m_ptr--; // Push the tag back
            return ScriptValue();
        }
    }

    const uint8_t* m_ptr;
    const uint8_t* m_end;
    unsigned m_version;
    Vector<CachedString> m_constantPool;
    MessagePortArray* m_messagePorts;
    ArrayBufferContentsArray* m_arrayBufferContents;
    ArrayBufferArray m_arrayBuffers;
};

DeserializationResult CloneDeserializer::deserialize()
{
    Vector<uint32_t, 16> indexStack;
    Vector<String, 16> propertyNameStack;
    Vector<ScriptValue, 32> outputObjectStack;
    Vector<WalkerState, 16> stateStack;
    WalkerState state = StateUnknown;
    ScriptValue outValue;

    unsigned tickCount = ticksUntilNextCheck();
    while (1) {
        switch (state) {
        arrayStartState:
        case ArrayStartState: {
            uint32_t length;
            if (!read(length)) {
                fail();
                goto error;
            }
            ScriptValue outArray = m_delegate->constructEmptyArray(length);
            m_gcBuffer.append(outArray);
            outputObjectStack.append(outArray);
            // fallthrough
        }
        arrayStartVisitMember:
        case ArrayStartVisitMember: {
            if (!--tickCount) {
                if (didTimeOut())
                    return make_pair(ScriptValue(), InterruptedExecutionError);
                tickCount = ticksUntilNextCheck();
            }

            uint32_t index;
            if (!read(index)) {
                fail();
                goto error;
            }
            if (index == TerminatorTag) {
                ScriptValue outArray = outputObjectStack.last();
                outValue = outArray;
                outputObjectStack.removeLast();
                break;
            } else if (index == NonIndexPropertiesTag) {
                goto objectStartVisitMember;
            }
            
            ScriptValue terminal = readTerminal();
            if (!terminal.hasNoValue()) {
                putProperty(outputObjectStack.last(), index, terminal);
                goto arrayStartVisitMember;
            }
            if (m_failed)
                goto error;
            indexStack.append(index);
            stateStack.append(ArrayEndVisitMember);
            goto stateUnknown;
        }
        case ArrayEndVisitMember: {
            ScriptValue outArray = outputObjectStack.last();
            putProperty(outArray, indexStack.last(), outValue);
            indexStack.removeLast();
            goto arrayStartVisitMember;
        }
        objectStartState:
        case ObjectStartState: {
            if (outputObjectStack.size() > maximumFilterRecursion)
                return make_pair(ScriptValue(), StackOverflowError);
            ScriptValue outObject = m_delegate->constructEmptyObject();
            m_gcBuffer.append(outObject);
            outputObjectStack.append(outObject);
            // fallthrough
        }
        objectStartVisitMember:
        case ObjectStartVisitMember: {
            if (!--tickCount) {
                if (didTimeOut())
                    return make_pair(ScriptValue(), InterruptedExecutionError);
                tickCount = ticksUntilNextCheck();
            }

            CachedStringRef cachedString;
            bool wasTerminator = false;
            if (!readStringData(cachedString, wasTerminator)) {
                if (!wasTerminator)
                    goto error;

                ScriptValue outObject = outputObjectStack.last();
                outValue = outObject;
                outputObjectStack.removeLast();
                break;
            }

            ScriptValue terminal = readTerminal();
            if (!terminal.hasNoValue()) {
                putProperty(outputObjectStack.last(), cachedString->string(), terminal);
                goto objectStartVisitMember;
            }
            stateStack.append(ObjectEndVisitMember);
            propertyNameStack.append(cachedString->string());
            goto stateUnknown;
        }
        case ObjectEndVisitMember: {
            putProperty(outputObjectStack.last(), propertyNameStack.last(), outValue);
            propertyNameStack.removeLast();
            goto objectStartVisitMember;
        }
        stateUnknown:
        case StateUnknown:
            ScriptValue terminal = readTerminal();
            if (!terminal.hasNoValue()) {
                outValue = terminal;
                break;
            }
            SerializationTag tag = readTag();
            if (tag == ArrayTag)
                goto arrayStartState;
            if (tag == ObjectTag)
                goto objectStartState;
            goto error;
        }
        if (stateStack.isEmpty())
            break;

        state = stateStack.last();
        stateStack.removeLast();

        if (!--tickCount) {
            if (didTimeOut())
                return make_pair(ScriptValue(), InterruptedExecutionError);
            tickCount = ticksUntilNextCheck();
        }
    }
    ASSERT(!outValue.hasNoValue());
    ASSERT(!m_failed);
    return make_pair(outValue, SuccessfullyCompleted);
error:
    fail();
    return make_pair(ScriptValue(), ValidationError);
}



SerializedScriptValue::~SerializedScriptValue()
{
}

SerializedScriptValue::SerializedScriptValue(const Vector<uint8_t>& buffer)
    : m_data(buffer)
{
}

SerializedScriptValue::SerializedScriptValue(Vector<uint8_t>& buffer)
{
    m_data.swap(buffer);
}

SerializedScriptValue::SerializedScriptValue(Vector<uint8_t>& buffer, Vector<String>& blobURLs)
{
    m_data.swap(buffer);
    m_blobURLs.swap(blobURLs);
}

SerializedScriptValue::SerializedScriptValue(Vector<uint8_t>& buffer, Vector<String>& blobURLs, PassOwnPtr<ArrayBufferContentsArray> arrayBufferContentsArray)
    : m_arrayBufferContentsArray(arrayBufferContentsArray)
{
    m_data.swap(buffer);
    m_blobURLs.swap(blobURLs);
}

PassOwnPtr<SerializedScriptValue::ArrayBufferContentsArray> SerializedScriptValue::transferArrayBuffers(
    ArrayBufferArray& arrayBuffers, SerializationReturnCode& code)
{
    for (size_t i = 0; i < arrayBuffers.size(); i++) {
        if (arrayBuffers[i]->isNeutered()) {
            code = ValidationError;
            return nullptr;
        }
    }

    OwnPtr<ArrayBufferContentsArray> contents = adoptPtr(new ArrayBufferContentsArray(arrayBuffers.size()));

    HashSet<WTF::ArrayBuffer*> visited;
    for (size_t i = 0; i < arrayBuffers.size(); i++) {
        Vector<RefPtr<ArrayBufferView> > neuteredViews;

        if (visited.contains(arrayBuffers[i].get()))
            continue;
        visited.add(arrayBuffers[i].get());

        bool result = arrayBuffers[i]->transfer(contents->at(i), neuteredViews);
        if (!result) {
            code = ValidationError;
            return nullptr;
        }
    }
    return contents.release();
}

PassRefPtr<SerializedScriptValue> SerializedScriptValue::create(PassOwnPtr<SerializationDelegate> popDelegate, const ScriptValue& value,
                                                                MessagePortArray* messagePorts, ArrayBufferArray* arrayBuffers,
                                                                SerializationErrorMode throwExceptions)
{
    Vector<uint8_t> buffer;
    Vector<String> blobURLs;
    OwnPtr<SerializationDelegate> delegate = popDelegate;
    SerializationReturnCode code = CloneSerializer::serialize(delegate.get(), value, messagePorts, arrayBuffers, blobURLs, buffer);

    OwnPtr<ArrayBufferContentsArray> arrayBufferContentsArray;

    if (arrayBuffers && serializationDidCompleteSuccessfully(code))
        arrayBufferContentsArray = transferArrayBuffers(*arrayBuffers, code);

    if (throwExceptions == Throwing)
        delegate->maybeThrowExceptionIfSerializationFailed(code);

    if (!serializationDidCompleteSuccessfully(code))
        return 0;

    return adoptRef(new SerializedScriptValue(buffer, blobURLs, arrayBufferContentsArray.release()));
}


PassRefPtr<SerializedScriptValue> SerializedScriptValue::create(JSC::ExecState* exec, JSC::JSValue value,
                                                                MessagePortArray* messagePorts, ArrayBufferArray* arrayBuffers,
                                                                SerializationErrorMode throwExceptions)
{
    OwnPtr<JSSerializationDelegate> delegate = JSSerializationDelegate::create(exec);
    ScriptValue scriptValue = JSScriptValue::scriptValue(exec->globalData(), value);
    return create(delegate.release(), scriptValue, messagePorts, arrayBuffers, throwExceptions);
}

PassRefPtr<SerializedScriptValue> SerializedScriptValue::create()
{
    Vector<uint8_t> buffer;
    return adoptRef(new SerializedScriptValue(buffer));
}

PassRefPtr<SerializedScriptValue> SerializedScriptValue::create(const String& string)
{
    Vector<uint8_t> buffer;
    if (!CloneSerializer::serialize(string, buffer))
        return 0;
    return adoptRef(new SerializedScriptValue(buffer));
}

#if ENABLE(INDEXED_DATABASE)
PassRefPtr<SerializedScriptValue> SerializedScriptValue::create(JSC::ExecState* exec, JSC::JSValue value)
{
    return SerializedScriptValue::create(exec, value, 0, 0);
}

PassRefPtr<SerializedScriptValue> SerializedScriptValue::numberValue(double value)
{
    Vector<uint8_t> buffer;
    CloneSerializer::serializeNumber(value, buffer);
    return adoptRef(new SerializedScriptValue(buffer));
}

JSValue SerializedScriptValue::deserialize(JSC::ExecState* exec, JSC::JSGlobalObject* globalObject)
{
    return deserialize(JSSerializationDelegate::create(exec, globalObject), 0);
}
#endif

PassRefPtr<SerializedScriptValue> SerializedScriptValue::create(JSContextRef originContext, JSValueRef apiValue, 
                                                                MessagePortArray* messagePorts, ArrayBufferArray* arrayBuffers,
                                                                JSValueRef* exception)
{
    JSC::ExecState* exec = toJS(originContext);
    JSC::APIEntryShim entryShim(exec);
    JSC::JSValue value = toJS(exec, apiValue);
    RefPtr<SerializedScriptValue> serializedValue = SerializedScriptValue::create(exec, value, messagePorts, arrayBuffers);
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        return 0;
    }
    ASSERT(serializedValue);
    return serializedValue.release();
}

PassRefPtr<SerializedScriptValue> SerializedScriptValue::create(JSContextRef originContext, JSValueRef apiValue,
                                                                JSValueRef* exception)
{
    return create(originContext, apiValue, 0, 0, exception);
}

String SerializedScriptValue::toString()
{
    return CloneDeserializer::deserializeString(m_data);
}
    
ScriptValue SerializedScriptValue::deserialize(PassOwnPtr<SerializationDelegate> popDelegate,
                                               MessagePortArray* messagePorts, SerializationErrorMode throwExceptions)
{
    OwnPtr<SerializationDelegate> delegate = popDelegate;
    DeserializationResult result = CloneDeserializer::deserialize(delegate.get(), messagePorts,
                                                                  m_arrayBufferContentsArray.get(), m_data);
    if (throwExceptions == Throwing)
        delegate->maybeThrowExceptionIfSerializationFailed(result.second);
    return result.first;
}

JSC::JSValue SerializedScriptValue::deserialize(JSC::ExecState* exec, JSC::JSGlobalObject* globalObject,
                                           MessagePortArray* messagePorts, SerializationErrorMode throwExceptions)
{
    ScriptValue value = deserialize(JSSerializationDelegate::create(exec, globalObject), messagePorts, throwExceptions);
    return static_cast<JSScriptValue*>(value.delegate())->jsValue();
}

#if ENABLE(INSPECTOR)
ScriptValue SerializedScriptValue::deserializeForInspector(ScriptState* scriptState)
{
    OwnPtr<SerializationDelegate> delegate;
    switch (scriptState->scriptType()) {
    case JSScriptType: {
        JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
        delegate = JSSerializationDelegate::create(exec, exec->lexicalGlobalObject());
        break;
    }
        
    case RBScriptType: {
        delegate = RBSerializationDelegate::create();
        break;
    }
    }

    return deserialize(delegate.release(), 0);
}
#endif

JSValueRef SerializedScriptValue::deserialize(JSContextRef destinationContext, JSValueRef* exception, MessagePortArray* messagePorts)
{
    JSC::ExecState* exec = toJS(destinationContext);
    JSC::APIEntryShim entryShim(exec);
    JSC::JSValue value = deserialize(exec, exec->lexicalGlobalObject(), messagePorts);
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        return 0;
    }
    ASSERT(value);
    return toRef(exec, value);
}

JSValueRef SerializedScriptValue::deserialize(JSContextRef destinationContext, JSValueRef* exception)
{
    return deserialize(destinationContext, exception, 0);
}

PassRefPtr<SerializedScriptValue> SerializedScriptValue::nullValue()
{
    return SerializedScriptValue::create();
}

PassRefPtr<SerializedScriptValue> SerializedScriptValue::undefinedValue()
{
    Vector<uint8_t> buffer;
    CloneSerializer::serializeUndefined(buffer);
    return adoptRef(new SerializedScriptValue(buffer));
}

PassRefPtr<SerializedScriptValue> SerializedScriptValue::booleanValue(bool value)
{
    Vector<uint8_t> buffer;
    CloneSerializer::serializeBoolean(value, buffer);
    return adoptRef(new SerializedScriptValue(buffer));
}

bool SerializedScriptValue::serializationDidCompleteSuccessfully(SerializationReturnCode code)
{
    return (code == SuccessfullyCompleted);
}

uint32_t SerializedScriptValue::wireFormatVersion()
{
    return CurrentVersion;
}

}
