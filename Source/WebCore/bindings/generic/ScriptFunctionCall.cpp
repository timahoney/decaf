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
#include "ScriptFunctionCall.h"

#include "JSScriptFunctionCall.h"
#include "RBScriptFunctionCall.h"
#include "ScriptValue.h"

namespace WebCore {
    
ScriptCallArgumentHandler::ScriptCallArgumentHandler(ScriptState* state)
: m_state(state)
{
}
    
ScriptCallArgumentHandler::~ScriptCallArgumentHandler()
{
}

void ScriptCallArgumentHandler::appendArgument(const ScriptObject& argument)
{
    if (argument.scriptState() != m_state) {
        ASSERT_NOT_REACHED();
        return;
    }

    argumentDelegate()->appendArgument(argument);
}

void ScriptCallArgumentHandler::appendArgument(const ScriptValue& argument)
{
    argumentDelegate()->appendArgument(argument);
}

void ScriptCallArgumentHandler::appendArgument(const String& argument)
{
    argumentDelegate()->appendArgument(argument);
}

void ScriptCallArgumentHandler::appendArgument(const char* argument)
{
    argumentDelegate()->appendArgument(argument);
}

void ScriptCallArgumentHandler::appendArgument(long argument)
{
    argumentDelegate()->appendArgument(argument);
}

void ScriptCallArgumentHandler::appendArgument(long long argument)
{
    argumentDelegate()->appendArgument(argument);
}

void ScriptCallArgumentHandler::appendArgument(unsigned int argument)
{
    argumentDelegate()->appendArgument(argument);
}

void ScriptCallArgumentHandler::appendArgument(unsigned long argument)
{
    argumentDelegate()->appendArgument(argument);
}

void ScriptCallArgumentHandler::appendArgument(int argument)
{
    argumentDelegate()->appendArgument(argument);
}

void ScriptCallArgumentHandler::appendArgument(bool argument)
{
    argumentDelegate()->appendArgument(argument);
}
    
ScriptFunctionCall::ScriptFunctionCall(PassRefPtr<ScriptFunctionCallDelegate> delegate)
: ScriptCallArgumentHandler(delegate->m_state)
, m_delegate(delegate)
{
}

ScriptFunctionCall::ScriptFunctionCall(const ScriptObject& thisObject, const String& name)
: ScriptFunctionCall(ScriptFunctionCallDelegate::create(thisObject, name))
{
}
    
ScriptCallArgumentHandlerDelegate* ScriptFunctionCall::argumentDelegate() const
{
    return m_delegate.get();
}

PassRefPtr<ScriptFunctionCallDelegate> ScriptFunctionCallDelegate::create(const ScriptObject& object, const String& name)
{
    switch (object.scriptType()) {
    case JSScriptType:
        return JSScriptFunctionCall::create(object, name);
    case RBScriptType:
        return RBScriptFunctionCall::create(object, name);
    }
}

ScriptValue ScriptFunctionCall::call(bool& hadException, bool reportExceptions)
{
    return m_delegate->call(hadException, reportExceptions);
}

ScriptValue ScriptFunctionCall::call()
{
    bool hadException = false;
    return call(hadException);
}

ScriptObject ScriptFunctionCall::construct(bool& hadException, bool reportExceptions)
{
    return m_delegate->construct(hadException, reportExceptions);
}

ScriptCallback::ScriptCallback(PassRefPtr<ScriptCallbackDelegate> delegate)
: ScriptCallArgumentHandler(delegate->m_state)
, m_delegate(delegate)
{
}

ScriptCallback::ScriptCallback(ScriptState* state, const ScriptValue& function)
: ScriptCallback(ScriptCallbackDelegate::create(state, function))
{
}

ScriptCallArgumentHandlerDelegate* ScriptCallback::argumentDelegate() const
{
    return m_delegate.get();
}

PassRefPtr<ScriptCallbackDelegate> ScriptCallbackDelegate::create(ScriptState* state, const ScriptValue& function)
{
    switch (state->scriptType()) {
        case JSScriptType:
            return JSScriptCallback::create(state, function);
        case RBScriptType:
            return RBScriptCallback::create(state, function);
    }
}

ScriptCallArgumentHandlerDelegate::ScriptCallArgumentHandlerDelegate(ScriptState* state)
: m_state(state)
{
}

ScriptCallArgumentHandlerDelegate::~ScriptCallArgumentHandlerDelegate()
{
}

ScriptValue ScriptCallback::call()
{
    return m_delegate->call();
}
    
ScriptFunctionCallDelegate::~ScriptFunctionCallDelegate()
{
}

ScriptFunctionCallDelegate::ScriptFunctionCallDelegate(const ScriptObject& thisObject, const String& name)
: ScriptCallArgumentHandlerDelegate(thisObject.scriptState())
, m_thisObject(thisObject)
, m_name(name)
{
}

ScriptCallbackDelegate::~ScriptCallbackDelegate()
{
}
    
ScriptCallbackDelegate::ScriptCallbackDelegate(ScriptState* state, const ScriptValue& function)
: ScriptCallArgumentHandlerDelegate(state)
, m_function(function)
{
}

} // namespace WebCore
