/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSSerializationDelegate.h"

#include "ExceptionCode.h"
#include "JSArrayBuffer.h"
#include "JSArrayBufferView.h"
#include "JSBlob.h"
#include "JSDataView.h"
#include "JSDOMGlobalObject.h"
#include "JSFile.h"
#include "JSFileList.h"
#include "JSFloat32Array.h"
#include "JSFloat64Array.h"
#include "JSImageData.h"
#include "JSInt16Array.h"
#include "JSInt32Array.h"
#include "JSInt8Array.h"
#include "JSMessagePort.h"
#include "JSNavigator.h"
#include "JSScriptState.h"
#include "JSScriptValue.h"
#include "JSUint16Array.h"
#include "JSUint32Array.h"
#include "JSUint8Array.h"
#include "JSUint8ClampedArray.h"
#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/APIShims.h>
#include <runtime/BooleanObject.h>
#include <runtime/DateInstance.h>
#include <runtime/Error.h>
#include <runtime/ExceptionHelpers.h>
#include <runtime/ObjectConstructor.h>
#include <runtime/Operations.h>
#include <runtime/PropertyNameArray.h>
#include <runtime/RegExp.h>
#include <runtime/RegExpObject.h>

using namespace JSC;

namespace WebCore {

JSSerializationDelegate::JSSerializationDelegate(ExecState* exec, JSGlobalObject* globalObject)
    : SerializationDelegate(JSScriptType)
    , m_exec(exec)
    , m_globalObject(globalObject)
    , m_isDOMGlobalObject(globalObject && globalObject->inherits(&JSDOMGlobalObject::s_info))
    , m_timeoutChecker(exec->globalData().timeoutChecker)
{
}

JSSerializationDelegate::~JSSerializationDelegate()
{
}

JSSerializationDelegate::JSSerializationDelegate(ExecState* exec)
    : JSSerializationDelegate(exec, 0)
{
}

bool JSSerializationDelegate::shouldTerminate()
{
    return m_exec->hadException();
}

unsigned JSSerializationDelegate::ticksUntilNextCheck()
{
    return m_timeoutChecker.ticksUntilNextCheck();
}

bool JSSerializationDelegate::didTimeOut()
{
    return m_timeoutChecker.didTimeOut(m_exec);
}

void JSSerializationDelegate::throwStackOverflow()
{
    throwError(m_exec, createStackOverflowError(m_exec));
}

void JSSerializationDelegate::throwInterruptedException()
{
    throwError(m_exec, createInterruptedExecutionException(&m_exec->globalData()));
}

void JSSerializationDelegate::throwValidationError()
{
    throwError(m_exec, createTypeError(m_exec, "Unable to deserialize data."));
}

void JSSerializationDelegate::maybeThrowExceptionIfSerializationFailed(SerializationReturnCode code)
{
    if (code == SuccessfullyCompleted)
        return;
    
    switch (code) {
    case StackOverflowError:
        throwError(m_exec, createStackOverflowError(m_exec));
        break;
    case InterruptedExecutionError:
        throwError(m_exec, createInterruptedExecutionException(&m_exec->globalData()));
        break;
    case ValidationError:
        throwError(m_exec, createTypeError(m_exec, "Unable to deserialize data."));
        break;
    case DataCloneError:
        setDOMException(m_exec, DATA_CLONE_ERR);
        break;
    case ExistingExceptionError:
        break;
    case UnspecifiedError:
        break;
    default:
        ASSERT_NOT_REACHED();
    }
}

template <class T>
ScriptValue toScriptValueInternal(ExecState* exec, T* nativeValue)
{
    JSDOMGlobalObject* globalObject = jsCast<JSDOMGlobalObject*>(exec->lexicalGlobalObject());
    JSC::JSValue value = toJS(exec, globalObject, nativeValue);
    JSC::JSObject* obj = value.getObject();
    if (!obj)
        return ScriptValue();

    return JSScriptValue::scriptValue(exec->globalData(), obj);
}

ScriptValue JSSerializationDelegate::toScriptValue(MessagePort* messagePort)
{
    return toScriptValueInternal(m_exec, messagePort);
}

ScriptValue JSSerializationDelegate::toScriptValue(ArrayBuffer* arrayBuffer)
{
    return toScriptValueInternal(m_exec, arrayBuffer);
}

ScriptValue JSSerializationDelegate::toScriptValue(DataView* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Int8Array* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Uint8Array* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Uint8ClampedArray* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Int16Array* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Uint16Array* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Int32Array* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Uint32Array* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Float32Array* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Float64Array* array)
{
    JSValue value = getJSValue(array);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(File* file)
{
    if (!m_isDOMGlobalObject)
        return null();

    JSValue value = toJS(m_exec, jsCast<JSDOMGlobalObject*>(m_globalObject), file);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(FileList* fileList)
{
    if (!m_isDOMGlobalObject)
        return null();
    JSValue value = getJSValue(fileList);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(ImageData* imageData)
{
    if (!m_isDOMGlobalObject)
        return null();
    JSValue value = getJSValue(imageData);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(Blob* blob)
{
    if (!m_isDOMGlobalObject)
        return null();
    JSValue value = getJSValue(blob);
    return JSScriptValue::scriptValue(m_exec->globalData(), value);
}

ScriptValue JSSerializationDelegate::toScriptValue(const String& string)
{
    return JSScriptValue::scriptValue(m_exec->globalData(), jsString(m_exec, string));
}

ScriptValue JSSerializationDelegate::null()
{
    return JSScriptValue::scriptValue(m_exec->globalData(), jsNull());
}

ScriptValue JSSerializationDelegate::undefined()
{
    return JSScriptValue::scriptValue(m_exec->globalData(), jsUndefined());
}

ScriptValue JSSerializationDelegate::emptyString()
{
    return JSScriptValue::scriptValue(m_exec->globalData(), jsEmptyString(&m_exec->globalData()));
}

ScriptValue JSSerializationDelegate::toScriptValue(double d)
{
    return JSScriptValue::scriptValue(m_exec->globalData(), jsNumber(d));
}

ScriptValue JSSerializationDelegate::toScriptValue(int64_t i)
{
    return JSScriptValue::scriptValue(m_exec->globalData(), jsNumber(i));
}

ScriptValue JSSerializationDelegate::toScriptValue(bool value)
{
    return JSScriptValue::scriptValue(m_exec->globalData(), jsBoolean(value));
}

ScriptValue JSSerializationDelegate::toRegExpScriptValue(const String& flags, const String& pattern)
{
    RegExpFlags reFlags = regExpFlags(flags);
    ASSERT(reFlags != InvalidFlags);
    RegExp* regExp = RegExp::create(m_exec->globalData(), pattern, reFlags);
    RegExpObject* object = RegExpObject::create(m_exec, m_exec->lexicalGlobalObject(), m_globalObject->regExpStructure(), regExp);
    return JSScriptValue::scriptValue(m_exec->globalData(), object);
}

ScriptValue JSSerializationDelegate::toDateScriptValue(double time)
{
    DateInstance* object = DateInstance::create(m_exec, m_globalObject->dateStructure(), time);
    return JSScriptValue::scriptValue(m_exec->globalData(), object);
}

ScriptValue JSSerializationDelegate::toBooleanObjectScriptValue(bool value)
{
    BooleanObject* obj = BooleanObject::create(m_exec->globalData(), m_globalObject->booleanObjectStructure());
    obj->setInternalValue(m_exec->globalData(), jsBoolean(value));
    return JSScriptValue::scriptValue(m_exec->globalData(), obj);
}

ScriptValue JSSerializationDelegate::toNumberObjectScriptValue(double d)
{
    NumberObject* obj = constructNumber(m_exec, m_globalObject, jsNumber(d));
    return JSScriptValue::scriptValue(m_exec->globalData(), obj);
}

ScriptValue JSSerializationDelegate::toStringObjectScriptValue(const String& value)
{
    StringObject* obj = constructString(m_exec, m_globalObject, jsString(&m_exec->globalData(), value));
    return JSScriptValue::scriptValue(m_exec->globalData(), obj);
}

bool JSSerializationDelegate::isArray(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    if (!value.isObject())
        return false;
    JSObject* object = asObject(value);
    return isJSArray(object) || object->inherits(&JSArray::s_info);
}

unsigned JSSerializationDelegate::arrayLength(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return asArray(value)->length();
}

ScriptValue JSSerializationDelegate::getProperty(const ScriptValue& scriptValue, const String& name)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    JSObject* object = asObject(value);
    Identifier propertyName(m_exec, name);
    PropertySlot slot(object);
    if (object->methodTable()->getOwnPropertySlot(object, m_exec, propertyName, slot))
        return JSScriptValue::scriptValue(m_exec->globalData(), slot.getValue(m_exec, propertyName));
    return ScriptValue();
}

bool JSSerializationDelegate::isDate(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return asObject(value)->inherits(&DateInstance::s_info);
}

double JSSerializationDelegate::asDateValue(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return asDateInstance(value)->internalNumber();
}

SerializationTag JSSerializationDelegate::tagForObject(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    JSObject* obj = asObject(value);
    if (obj->inherits(&BooleanObject::s_info))
        return asBooleanObject(value)->internalValue().toBoolean(m_exec) ? TrueObjectTag : FalseObjectTag;
    if (obj->inherits(&StringObject::s_info))
        return StringObjectTag;
    if (obj->inherits(&NumberObject::s_info))
        return NumberObjectTag;
    if (obj->inherits(&JSFile::s_info))
        return FileTag;
    if (obj->inherits(&JSFileList::s_info))
        return FileListTag;
    if (obj->inherits(&JSBlob::s_info))
        return BlobTag;
    if (obj->inherits(&JSImageData::s_info))
        return ImageDataTag;
    if (obj->inherits(&RegExpObject::s_info))
        return RegExpTag;
    if (obj->inherits(&JSMessagePort::s_info))
        return MessagePortReferenceTag;
    if (obj->inherits(&JSArrayBuffer::s_info))
        return ArrayBufferTag;
    if (obj->inherits(&JSArrayBufferView::s_info))
        return ArrayBufferViewTag;
    return ErrorTag;
}

double JSSerializationDelegate::asNumber(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    NumberObject* obj = static_cast<NumberObject*>(asObject(value));
    return obj->internalValue().asNumber();
}

String JSSerializationDelegate::asString(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return value.toString(m_exec)->value(m_exec);
}

File* JSSerializationDelegate::toFile(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return ::WebCore::toFile(value);
}

FileList* JSSerializationDelegate::toFileList(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return ::WebCore::toFileList(value);
}

Blob* JSSerializationDelegate::toBlob(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return ::WebCore::toBlob(value);
}

ImageData* JSSerializationDelegate::toImageData(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return ::WebCore::toImageData(value);
}

bool JSSerializationDelegate::isRegExpGlobal(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return asRegExpObject(value)->regExp()->global();
}

bool JSSerializationDelegate::isRegExpIgnoreCase(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return asRegExpObject(value)->regExp()->ignoreCase();
}

bool JSSerializationDelegate::isRegExpMultiline(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return asRegExpObject(value)->regExp()->multiline();
}

String JSSerializationDelegate::asRegExpPattern(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return asRegExpObject(value)->regExp()->pattern();
}

ArrayBuffer* JSSerializationDelegate::toArrayBuffer(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return ::WebCore::toArrayBuffer(value);
}

ArrayBufferView* JSSerializationDelegate::toArrayBufferView(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    return ::WebCore::toArrayBufferView(value);
}

void JSSerializationDelegate::setArrayElement(const ScriptValue& objectScriptValue, unsigned index, const ScriptValue& elementScriptValue)
{
    JSValue objectValue = static_cast<JSScriptValue*>(objectScriptValue.delegate())->jsValue();
    JSObject* object = asObject(objectValue);
    JSValue element = static_cast<JSScriptValue*>(elementScriptValue.delegate())->jsValue();
    object->putDirectIndex(m_exec, index, element);
}

void JSSerializationDelegate::setObjectProperty(const ScriptValue& objectScriptValue, const String& propertyName, const ScriptValue& propertyScriptValue)
{
    JSValue objectValue = static_cast<JSScriptValue*>(objectScriptValue.delegate())->jsValue();
    JSObject* object = asObject(objectValue);
    JSValue property = static_cast<JSScriptValue*>(propertyScriptValue.delegate())->jsValue();
    object->putDirectMayBeIndex(m_exec, Identifier(m_exec, propertyName), property);
}

static void movePropertyNameArrayToStringVector(PropertyNameArray& propertyNames, Vector<String>& strings)
{
    PropertyNameArrayData::PropertyNameVector& propertyNameVector = propertyNames.data()->propertyNameVector();
    for (PropertyNameArrayData::PropertyNameVector::const_iterator it = propertyNameVector.begin(); it != propertyNameVector.end(); it++)
        strings.append(it->string());
}

void JSSerializationDelegate::getOwnPropertyNames(const ScriptValue& scriptValue, Vector<String>& strings)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    JSObject* object = asObject(value);
    PropertyNameArray propertyNames(m_exec);
    object->methodTable()->getOwnPropertyNames(object, m_exec, propertyNames, ExcludeDontEnumProperties);
    movePropertyNameArrayToStringVector(propertyNames, strings);
}

void JSSerializationDelegate::getOwnNonIndexPropertyNames(const ScriptValue& scriptValue, Vector<String>& strings)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    JSArray* array = asArray(value);
    PropertyNameArray propertyNames(m_exec);
    array->methodTable()->getOwnNonIndexPropertyNames(array, m_exec, propertyNames, ExcludeDontEnumProperties);
    movePropertyNameArrayToStringVector(propertyNames, strings);
}

ScriptValue JSSerializationDelegate::objectAtIndex(const ScriptValue& scriptValue, unsigned index)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    JSArray* array = asArray(value);
    JSValue objectAtIndex = array->getDirectIndex(m_exec, index);
    if (!objectAtIndex)
        return ScriptValue();
    return JSScriptValue::scriptValue(m_exec->globalData(), objectAtIndex);
}

bool JSSerializationDelegate::isFinalObject(const ScriptValue& scriptValue)
{
    JSValue value = static_cast<JSScriptValue*>(scriptValue.delegate())->jsValue();
    JSObject* object = asObject(value);
    return object->classInfo() == &JSFinalObject::s_info;
}

ScriptValue JSSerializationDelegate::constructEmptyArray(unsigned length)
{
    JSArray* array = JSC::constructEmptyArray(m_exec, 0, m_globalObject, length);
    return JSScriptValue::scriptValue(m_exec->globalData(), array);
}

ScriptValue JSSerializationDelegate::constructEmptyObject()
{
    JSObject* object = JSC::constructEmptyObject(m_exec, m_globalObject->objectPrototype());
    return JSScriptValue::scriptValue(m_exec->globalData(), object);
}

} // namespace WebCore
