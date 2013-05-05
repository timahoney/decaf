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
 * THIS SOFTWARE IS PROVIDED BY APPLE, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ScriptValue.h"

#include "InspectorValues.h"
#include "JSScriptValue.h"
#include "RBScriptValue.h"
#include "SerializedScriptValue.h"
#include <heap/Strong.h>
#include <heap/StrongInlines.h>
#include <runtime/JSCJSValue.h>
#include <runtime/Operations.h>

namespace WebCore {

ScriptValue::ScriptValue()
    : m_delegate(EmptyScriptValueDelegate::create())
{
}

ScriptValue::ScriptValue(PassRefPtr<ScriptValueDelegate> delegate)
    : m_delegate(delegate)
{
}

ScriptValue::ScriptValue(JSC::JSGlobalData& globalData, JSC::JSValue value)
    : ScriptValue(JSScriptValue::create(globalData, value))
{
}

ScriptValue::ScriptValue(VALUE value)
    : m_delegate(RBScriptValue::create(value))
{
}

bool ScriptValue::isString() const
{
    return m_delegate->isString();
}

bool ScriptValue::getString(ScriptState* scriptState, String& result) const
{
    if (scriptState->scriptType() != scriptType())
        return false;

    return m_delegate->getString(scriptState, result);
}

String ScriptValue::toString(ScriptState* scriptState) const
{
    if (scriptState->scriptType() != scriptType())
        return "ERROR: Trying to get a String from a ScriptValue from a different language.";
    
    return m_delegate->toString(scriptState);
}

bool ScriptValue::isEqual(ScriptState* scriptState, const ScriptValue& anotherValue) const
{
    if (scriptState->scriptType() != scriptType())
        return false;
    
    return m_delegate->isEqual(scriptState, anotherValue);
}

bool ScriptValue::isNull() const
{
    return m_delegate->isNull();
}

bool ScriptValue::isUndefined() const
{
    return m_delegate->isUndefined();
}

bool ScriptValue::isObject() const
{
    return m_delegate->isObject();
}

bool ScriptValue::isFunction() const
{
    return m_delegate->isFunction();
}

bool ScriptValue::hasNoValue() const
{
    return m_delegate->hasNoValue();
}
    
JSC::JSValue ScriptValue::jsValue() const
{
    ASSERT(scriptType() == JSScriptType);
    if (scriptType() != JSScriptType)
        return JSC::jsNull();
    return static_cast<JSScriptValue*>(delegate())->jsValue();
}

VALUE ScriptValue::rbValue() const
{
    ASSERT(scriptType() == RBScriptType);
    if (scriptType() != RBScriptType)
        return Qnil;
    return static_cast<RBScriptValue*>(delegate())->rbValue();
}

void ScriptValue::clear()
{
    m_delegate->clear();
}

bool ScriptValue::isNumber() const
{
    return m_delegate->isNumber();
}

bool ScriptValue::isInt32() const
{
    return m_delegate->isInt32();
}

int32_t ScriptValue::asInt32() const
{
    return m_delegate->asInt32();
}

double ScriptValue::asDouble() const
{
    return m_delegate->asDouble();
}

double ScriptValue::asNumber() const
{
    return m_delegate->asNumber();
}

bool ScriptValue::isBoolean() const
{
    return m_delegate->isBoolean();
}

bool ScriptValue::isTrue() const
{
    return m_delegate->isTrue();
}

bool ScriptValue::isCell() const
{
    return m_delegate->isCell();
}

PassRefPtr<SerializedScriptValue> ScriptValue::serialize(ScriptState* scriptState, SerializationErrorMode throwExceptions)
{
    if (scriptState->scriptType() != scriptType())
        return SerializedScriptValue::create("ERROR: Serializing a ScriptValue from a different language.");
    
    return m_delegate->serialize(scriptState, throwExceptions);
}

PassRefPtr<SerializedScriptValue> ScriptValue::serialize(ScriptState* scriptState, MessagePortArray* messagePorts, ArrayBufferArray* arrayBuffers, bool& didThrow)
{
    if (scriptState->scriptType() != scriptType())
        return SerializedScriptValue::create("ERROR: Serializing a ScriptValue from a different language.");
    
    return m_delegate->serialize(scriptState, messagePorts, arrayBuffers, didThrow);
}

bool ScriptValue::operator==(const ScriptValue& other) const
{
    return *m_delegate == *other.m_delegate;
}

#if ENABLE(INSPECTOR)
PassRefPtr<InspectorValue> ScriptValue::toInspectorValue(ScriptState* scriptState) const
{
    if (scriptState->scriptType() != scriptType())
        return InspectorString::create("ERROR: Trying to create InspectorValue with ScriptValue from a different language.");
    
    return m_delegate->toInspectorValue(scriptState);
}
#endif // ENABLE(INSPECTOR)

} // namespace WebCore
