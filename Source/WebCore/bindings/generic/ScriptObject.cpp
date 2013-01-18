/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ScriptObject.h"

#include "JSDOMBinding.h"
#include "JSScriptState.h"

#include <runtime/JSLock.h>

#if ENABLE(INSPECTOR)
#include "JSInjectedScriptHost.h"
#include "JSInspectorFrontendHost.h"
#endif

using namespace JSC;

namespace WebCore {

ScriptObject::ScriptObject(ScriptState* scriptState, JSObject* object)
    : ScriptValue(static_cast<JSScriptState*>(scriptState)->execState()->globalData(), object)
    , m_scriptState(scriptState)
{
}

// FIXME: Make this constructor work for a generic ScriptValue.
ScriptObject::ScriptObject(ScriptState* scriptState, const ScriptValue& scriptValue)
    : ScriptValue(static_cast<JSScriptState*>(scriptState)->execState()->globalData(), scriptValue.jsValue())
    , m_scriptState(scriptState)
{
}
    
ScriptObject::ScriptObject(ScriptState* scriptState, PassRefPtr<ScriptValueDelegate> delegate)
    : ScriptValue(delegate)
    , m_scriptState(scriptState)
{
}

static bool handleException(ScriptState* scriptState)
{
    if (!scriptState->hadException())
        return true;
    
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();

    reportException(exec, exec->exception());
    return false;
}

bool ScriptGlobalObject::set(ScriptState* scriptState, const char* name, const ScriptObject& value)
{
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    JSLockHolder lock(exec);
    exec->lexicalGlobalObject()->putDirect(exec->globalData(), Identifier(exec, name), value.jsObject());
    return handleException(scriptState);
}

#if ENABLE(INSPECTOR)
bool ScriptGlobalObject::set(ScriptState* scriptState, const char* name, InspectorFrontendHost* value)
{
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    JSLockHolder lock(exec);
    JSDOMGlobalObject* globalObject = jsCast<JSDOMGlobalObject*>(exec->lexicalGlobalObject());
    globalObject->putDirect(exec->globalData(), Identifier(exec, name), toJS(exec, globalObject, value));
    return handleException(scriptState);
}

bool ScriptGlobalObject::set(ScriptState* scriptState, const char* name, InjectedScriptHost* value)
{
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    JSLockHolder lock(exec);
    JSDOMGlobalObject* globalObject = jsCast<JSDOMGlobalObject*>(exec->lexicalGlobalObject());
    globalObject->putDirect(exec->globalData(), Identifier(exec, name), toJS(exec, globalObject, value));
    return handleException(scriptState);
}
#endif // ENABLE(INSPECTOR)

bool ScriptGlobalObject::get(ScriptState* scriptState, const char* name, ScriptObject& value)
{
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    
    JSLockHolder lock(exec);
    JSValue jsValue = exec->lexicalGlobalObject()->get(exec, Identifier(exec, name));
    if (!jsValue)
        return false;

    if (!jsValue.isObject())
        return false;

    value = ScriptObject(scriptState, asObject(jsValue));
    return true;
}

bool ScriptGlobalObject::remove(ScriptState* scriptState, const char* name)
{
    JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState();
    
    JSLockHolder lock(exec);
    exec->lexicalGlobalObject()->methodTable()->deleteProperty(exec->lexicalGlobalObject(), exec, Identifier(exec, name));
    return handleException(scriptState);
}

} // namespace WebCore
