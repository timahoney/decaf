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
#include "RBScriptFunctionCall.h"

#include "RBCallHelpers.h"
#include "RBDOMBinding.h"
#include "RBScriptValue.h"
#include <wtf/text/WTFString.h>

using namespace RB;

namespace WebCore {

RBScriptCallArgumentHandler::RBScriptCallArgumentHandler(ScriptState* state)
    : ScriptCallArgumentHandlerDelegate(state)
{
}

RBScriptCallArgumentHandler::~RBScriptCallArgumentHandler()
{
}

inline void RBScriptCallArgumentHandler::appendRBArgument(VALUE argument)
{
    appendArgument(RBScriptValue::scriptValue(argument));
}

void RBScriptCallArgumentHandler::appendArgument(const ScriptObject& argument)
{
    m_arguments.append(argument);
}

void RBScriptCallArgumentHandler::appendArgument(const ScriptValue& argument)
{
    m_arguments.append(argument);
}

static ScriptValue makeFunctionCall(ScriptState* scriptState, VALUE object, const String& functionName, const Vector<ScriptValue>& arguments, bool& hadException, bool reportExceptions)
{
    // Make the function call within the binding of the script state.
    RBScriptState* state = static_cast<RBScriptState*>(scriptState);
    VALUE createProc = rb_str_new2("Proc.new { |object, function_name, arguments| object.method(function_name).call(*arguments) }");
    VALUE proc = rb_funcall(state->binding(), rb_intern("eval"), 1, createProc);
    VALUE functionNameRB = rb_str_new2(functionName.utf8().data());
    VALUE argumentsRB = rb_ary_new2(arguments.size());
    for (size_t i = 0; i < arguments.size(); i++) {
        VALUE argument = static_cast<RBScriptValue*>(arguments[i].delegate())->rbValue();
        rb_ary_push(argumentsRB, argument);
    }
    
    VALUE exception;
    VALUE result = callFunction(proc, "call", object, functionNameRB, argumentsRB, &exception);
    if (!NIL_P(exception)) {
        if (reportExceptions)
            RBDOMBinding::reportException(state->scriptExecutionContext(), exception);
        
        hadException = true;
        return ScriptValue();
    }
    
    return RBScriptValue::scriptValue(result);
}

ScriptValue RBScriptFunctionCall::call(bool& hadException, bool reportExceptions)
{
    VALUE object = static_cast<RBScriptValue*>(m_thisObject.delegate())->rbValue();
    return makeFunctionCall(m_state, object, m_name, m_arguments, hadException, reportExceptions);
}

ScriptObject RBScriptFunctionCall::construct(bool&, bool)
{
    // FIXME: Implement this...if it's ever used.
    return ScriptObject();
}

ScriptValue RBScriptCallback::call()
{
    VALUE proc = static_cast<RBScriptValue*>(m_function.delegate())->rbValue();
    bool hadExceptions = false;
    return makeFunctionCall(m_state, proc, "call", m_arguments, hadExceptions, true);
}

} // namespace WebCore
