/*
 * Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
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

#ifndef SerializationDelegate_h
#define SerializationDelegate_h

#include "ScriptType.h"
#include "SerializationTags.h"
#include "SerializedScriptValue.h"

namespace WebCore {

class DataView;
class File;
class FileList;
class ImageData;
class Blob;

class SerializationDelegate {
public:
    virtual ~SerializationDelegate() {}
    
    virtual bool shouldTerminate() = 0;
    virtual void throwStackOverflow() = 0;
    virtual void throwInterruptedException() = 0;
    virtual void throwValidationError() = 0;

    // FIXME: Remove these and don't use them.
    virtual unsigned ticksUntilNextCheck() = 0;
    virtual bool didTimeOut() = 0;

    virtual void maybeThrowExceptionIfSerializationFailed(SerializationReturnCode) = 0;

    virtual SerializationTag tagForObject(const ScriptValue&) = 0;

    virtual ScriptValue toScriptValue(MessagePort*) = 0;
    virtual ScriptValue toScriptValue(ArrayBuffer*) = 0;
    virtual ScriptValue toScriptValue(File*) = 0;
    virtual ScriptValue toScriptValue(FileList*) = 0;
    virtual ScriptValue toScriptValue(ImageData*) = 0;
    virtual ScriptValue toScriptValue(Blob*) = 0;
    virtual ScriptValue toScriptValue(DataView*) = 0;
    virtual ScriptValue toScriptValue(Int8Array*) = 0;
    virtual ScriptValue toScriptValue(Uint8Array*) = 0;
    virtual ScriptValue toScriptValue(Uint8ClampedArray*) = 0;
    virtual ScriptValue toScriptValue(Int16Array*) = 0;
    virtual ScriptValue toScriptValue(Uint16Array*) = 0;
    virtual ScriptValue toScriptValue(Int32Array*) = 0;
    virtual ScriptValue toScriptValue(Uint32Array*) = 0;
    virtual ScriptValue toScriptValue(Float32Array*) = 0;
    virtual ScriptValue toScriptValue(Float64Array*) = 0;
    virtual ScriptValue toScriptValue(double d) = 0;
    virtual ScriptValue toScriptValue(int32_t i) = 0;
    virtual ScriptValue toScriptValue(bool value) = 0;
    virtual ScriptValue toScriptValue(const String&) = 0;
    virtual ScriptValue toRegExpScriptValue(const String& flags, const String& pattern) = 0;
    virtual ScriptValue toDateScriptValue(double time) = 0;

    // FIXME: Can we remove these?
    // Why does JavaScript have NumberObjects and BooleanObjects?
    virtual ScriptValue toBooleanObjectScriptValue(bool) = 0;
    virtual ScriptValue toNumberObjectScriptValue(double) = 0;
    virtual ScriptValue toStringObjectScriptValue(const String&) = 0;

    // FIXME: Should these be in ScriptValue instead?
    virtual ScriptValue null() = 0;
    virtual ScriptValue undefined() = 0;
    virtual ScriptValue emptyString() = 0;

    // FIXME: Should these be in ScriptValue instead?
    virtual bool isArray(const ScriptValue&) = 0;
    virtual bool isDate(const ScriptValue&) = 0;
    virtual double asDateValue(const ScriptValue&) = 0;
    virtual double asNumber(const ScriptValue&) = 0;
    virtual String asString(const ScriptValue&) = 0;
    virtual File* toFile(const ScriptValue&) = 0;
    virtual FileList* toFileList(const ScriptValue&) = 0;
    virtual Blob* toBlob(const ScriptValue&) = 0;
    virtual ImageData* toImageData(const ScriptValue&) = 0;
    virtual ArrayBuffer* toArrayBuffer(const ScriptValue&) = 0;
    virtual ArrayBufferView* toArrayBufferView(const ScriptValue&) = 0;

    // FIXME: Should these be in ScriptValue instead?
    virtual bool isRegExpGlobal(const ScriptValue&) = 0;
    virtual bool isRegExpIgnoreCase(const ScriptValue&) = 0;
    virtual bool isRegExpMultiline(const ScriptValue&) = 0;
    virtual String asRegExpPattern(const ScriptValue&) = 0;

    // FIXME: Should these be in ScriptValue instead?
    virtual void setArrayElement(const ScriptValue& object, unsigned index, const ScriptValue& value) = 0;
    virtual void setObjectProperty(const ScriptValue& object, const String& name, const ScriptValue& value) = 0;

    // FIXME: Should these be in ScriptValue instead?
    virtual void getOwnPropertyNames(const ScriptValue&, Vector<String>&) = 0;
    virtual void getOwnNonIndexPropertyNames(const ScriptValue&, Vector<String>&) = 0;
    virtual ScriptValue objectAtIndex(const ScriptValue&, unsigned i) = 0;
    virtual unsigned arrayLength(const ScriptValue&) = 0;
    virtual ScriptValue getProperty(const ScriptValue&, const String& name) = 0;
    virtual bool isFinalObject(const ScriptValue&) = 0;

    // FIXME: Should these be in ScriptValue instead?
    virtual ScriptValue constructEmptyArray(unsigned length = 0) = 0;
    virtual ScriptValue constructEmptyObject() = 0;
    
    ScriptType scriptType() const { return m_scriptType; }

protected:
    SerializationDelegate(ScriptType scriptType)
        : m_scriptType(scriptType)
    {
    }

private:
    ScriptType m_scriptType;
};

} // namespace WebCore

#endif // SerializationDelegate_h
