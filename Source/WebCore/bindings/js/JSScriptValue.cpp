/*
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (c) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSScriptValue.h"

#include "InspectorValues.h"
#include "JSDOMBinding.h"
#include "JSScriptState.h"
#include "SerializedScriptValue.h"

#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/JSValueRef.h>

#include <heap/Strong.h>
#include <runtime/JSLock.h>

using namespace JSC;

namespace WebCore {
    
JSScriptValue::~JSScriptValue()
{
}

bool JSScriptValue::isString() const
{
    if (!m_value)
        return false;

    return m_value.get().isString();
}

bool JSScriptValue::getString(ScriptState* scriptState, String& result) const
{
    if (!m_value)
        return false;
    
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    
    JSLockHolder lock(exec);
    if (!m_value.get().getString(exec, result))
        return false;
    return true;
}

String JSScriptValue::toString(ScriptState* scriptState) const
{
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    
    String result = m_value.get().toString(exec)->value(exec);
    // Handle the case where an exception is thrown as part of invoking toString on the object.
    if (scriptState->hadException())
        exec->clearException();
    return result;
}

bool JSScriptValue::isEqual(ScriptState* scriptState, const ScriptValue& anotherValue) const
{
    if (hasNoValue())
        return anotherValue.hasNoValue();
    
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();

    return JSValueIsEqual(toRef(exec), toRef(exec, jsValue()), toRef(exec, anotherValue.jsValue()), 0);
}

bool JSScriptValue::isNull() const
{
    if (!m_value)
        return false;
    return m_value.get().isNull();
}

bool JSScriptValue::isUndefined() const
{
    if (!m_value)
        return false;
    return m_value.get().isUndefined();
}

bool JSScriptValue::isObject() const
{
    if (!m_value)
        return false;
    return m_value.get().isObject();
}

bool JSScriptValue::isFunction() const
{
    CallData callData;
    return getCallData(m_value.get(), callData) != CallTypeNone;
}

bool JSScriptValue::isNumber() const
{
    if (!m_value)
        return false;
    return m_value.get().isNumber();
}

bool JSScriptValue::isInt32() const
{
    if (!m_value)
        return false;
    return m_value.get().isInt32();
}

int32_t JSScriptValue::asInt32() const
{
    if (!m_value)
        return 0;
    return m_value.get().asInt32();
}

double JSScriptValue::asDouble() const
{
    if (!m_value)
        return 0;
    return m_value.get().asDouble();
}

bool JSScriptValue::isBoolean() const
{
    if (!m_value)
        return false;
    return m_value.get().isBoolean();
}

bool JSScriptValue::isTrue() const
{
    if (!m_value)
        return false;
    return m_value.get().isTrue();
}

bool JSScriptValue::isCell() const
{
    if (!m_value)
        return false;
    return m_value.get().isCell();
}

bool JSScriptValue::operator==(const ScriptValueDelegate& other) const
{
    if (other.scriptType() != JSScriptType)
        return false;
    return m_value == static_cast<const JSScriptValue&>(other).m_value;
}

PassRefPtr<SerializedScriptValue> JSScriptValue::serialize(ScriptState* scriptState, SerializationErrorMode throwExceptions)
{
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    return SerializedScriptValue::create(exec, jsValue(), 0, 0, throwExceptions);
}

PassRefPtr<SerializedScriptValue> JSScriptValue::serialize(ScriptState* scriptState, MessagePortArray* messagePorts, ArrayBufferArray* arrayBuffers, bool& didThrow)
{
    JSValueRef exception = 0;
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    RefPtr<SerializedScriptValue> serializedValue = SerializedScriptValue::create(toRef(exec), toRef(exec, jsValue()), messagePorts, arrayBuffers, &exception);
    didThrow = exception ? true : false;
    return serializedValue.release();
}

#if ENABLE(INSPECTOR)
static PassRefPtr<InspectorValue> jsToInspectorValue(JSC::ExecState* exec, JSValue value, int maxDepth)
{
    if (!value) {
        ASSERT_NOT_REACHED();
        return 0;
    }

    if (!maxDepth)
        return 0;
    maxDepth--;

    if (value.isNull() || value.isUndefined())
        return InspectorValue::null();
    if (value.isBoolean())
        return InspectorBasicValue::create(value.asBoolean());
    if (value.isNumber())
        return InspectorBasicValue::create(value.asNumber());
    if (value.isString()) {
        String s = value.getString(exec);
        return InspectorString::create(String(s.characters(), s.length()));
    }
    if (value.isObject()) {
        if (isJSArray(value)) {
            RefPtr<InspectorArray> inspectorArray = InspectorArray::create();
            JSArray* array = asArray(value);
            unsigned length = array->length();
            for (unsigned i = 0; i < length; i++) {
                JSValue element = array->getIndex(exec, i);
                RefPtr<InspectorValue> elementValue = jsToInspectorValue(exec, element, maxDepth);
                if (!elementValue)
                    return 0;
                inspectorArray->pushValue(elementValue);
            }
            return inspectorArray;
        }
        RefPtr<InspectorObject> inspectorObject = InspectorObject::create();
        JSObject* object = value.getObject();
        PropertyNameArray propertyNames(exec);
        object->methodTable()->getOwnPropertyNames(object, exec, propertyNames, ExcludeDontEnumProperties);
        for (size_t i = 0; i < propertyNames.size(); i++) {
            const Identifier& name =  propertyNames[i];
            JSValue propertyValue = object->get(exec, name);
            RefPtr<InspectorValue> inspectorValue = jsToInspectorValue(exec, propertyValue, maxDepth);
            if (!inspectorValue)
                return 0;
            inspectorObject->setValue(String(name.characters(), name.length()), inspectorValue);
        }
        return inspectorObject;
    }
    ASSERT_NOT_REACHED();
    return 0;
}

PassRefPtr<InspectorValue> JSScriptValue::toInspectorValue(ScriptState* scriptState) const
{
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    JSC::JSLockHolder holder(exec);
    return jsToInspectorValue(exec, m_value.get(), InspectorValue::maxDepth);
}
#endif // ENABLE(INSPECTOR)

} // namespace WebCore
