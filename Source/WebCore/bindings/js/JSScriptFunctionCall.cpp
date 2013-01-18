/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
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
#include "JSScriptFunctionCall.h"

#include "JSDOMBinding.h"
#include "JSMainThreadExecState.h"
#include "JSScriptValue.h"
#include "ScriptValue.h"
#include <runtime/JSLock.h>
#include <wtf/text/WTFString.h>

using namespace JSC;

namespace WebCore {

void JSScriptCallArgumentHandler::appendArgument(const ScriptObject& argument)
{
    m_arguments.append(argument.jsObject());
}

void JSScriptCallArgumentHandler::appendArgument(const ScriptValue& argument)
{
    m_arguments.append(argument.jsValue());
}

void JSScriptCallArgumentHandler::appendArgument(const String& argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsStringWithCache(m_exec, argument));
}

void JSScriptCallArgumentHandler::appendArgument(const char* argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsString(m_exec, String(argument)));
}

void JSScriptCallArgumentHandler::appendArgument(long argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void JSScriptCallArgumentHandler::appendArgument(long long argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void JSScriptCallArgumentHandler::appendArgument(unsigned int argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void JSScriptCallArgumentHandler::appendArgument(unsigned long argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void JSScriptCallArgumentHandler::appendArgument(int argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void JSScriptCallArgumentHandler::appendArgument(bool argument)
{
    m_arguments.append(jsBoolean(argument));
}

static void createArgList(MarkedArgumentBuffer& arguments, Vector<JSC::JSValue>& vector)
{
    for (Vector<JSValue>::const_iterator it = vector.begin(); it != vector.end(); ++it)
        arguments.append(*it);
}

ScriptValue JSScriptFunctionCall::call(bool& hadException, bool reportExceptions)
{
    JSObject* thisObject = m_thisObject.jsObject();

    JSLockHolder lock(m_exec);

    JSValue function = thisObject->get(m_exec, Identifier(m_exec, m_name));
    if (m_exec->hadException()) {
        if (reportExceptions)
            reportException(m_exec, m_exec->exception());

        hadException = true;
        return ScriptValue();
    }

    CallData callData;
    CallType callType = getCallData(function, callData);
    if (callType == CallTypeNone)
        return ScriptValue();
    
    MarkedArgumentBuffer arguments;
    createArgList(arguments, m_arguments);

    JSValue result;
    if (isMainThread())
        result = JSMainThreadExecState::call(m_exec, function, callType, callData, thisObject, arguments);
    else
        result = JSC::call(m_exec, function, callType, callData, thisObject, arguments);

    if (m_exec->hadException()) {
        if (reportExceptions)
            reportException(m_exec, m_exec->exception());

        hadException = true;
        return ScriptValue();
    }

    return ScriptValue(m_exec->globalData(), result);
}

ScriptObject JSScriptFunctionCall::construct(bool& hadException, bool reportExceptions)
{
    JSObject* thisObject = m_thisObject.jsObject();

    JSLockHolder lock(m_exec);

    JSObject* constructor = asObject(thisObject->get(m_exec, Identifier(m_exec, m_name)));
    if (m_exec->hadException()) {
        if (reportExceptions)
            reportException(m_exec, m_exec->exception());

        hadException = true;
        return ScriptObject();
    }

    ConstructData constructData;
    ConstructType constructType = constructor->methodTable()->getConstructData(constructor, constructData);
    if (constructType == ConstructTypeNone)
        return ScriptObject();

    MarkedArgumentBuffer arguments;
    createArgList(arguments, m_arguments);
    JSValue result = JSC::construct(m_exec, constructor, constructType, constructData, arguments);
    if (m_exec->hadException()) {
        if (reportExceptions)
            reportException(m_exec, m_exec->exception());

        hadException = true;
        return ScriptObject();
    }

    JSScriptState* state = JSScriptState::forExecState(m_exec);
    return ScriptObject(state, JSScriptValue::scriptValue(m_exec->globalData(), asObject(result)));
}

ScriptValue JSScriptCallback::call()
{
    JSLockHolder lock(m_exec);

    CallData callData;
    CallType callType = getCallData(m_function.jsValue(), callData);
    if (callType == CallTypeNone)
        return ScriptValue();
    
    MarkedArgumentBuffer arguments;
    createArgList(arguments, m_arguments);

    JSValue result = JSC::call(m_exec, m_function.jsValue(), callType, callData, m_function.jsValue(), arguments);
    bool hadException = m_exec->hadException();

    if (hadException) {
        reportException(m_exec, m_exec->exception());
        return ScriptValue();
    }

    return ScriptValue(m_exec->globalData(), result);
}

} // namespace WebCore
