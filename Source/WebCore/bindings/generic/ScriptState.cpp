/*
 * Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
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
#include "ScriptState.h"

#include "JSScriptState.h"
#include "RBDOMBinding.h"
#include "RBScriptState.h"
#include <heap/Strong.h>
#include <heap/StrongInlines.h>
#include <interpreter/CallFrame.h>
#include <runtime/JSGlobalObject.h>
#include <runtime/Operations.h>

namespace WebCore {
    
ScriptState::ScriptState(ScriptType type)
    : m_type(type)
{
}
    
ScriptState::~ScriptState()
{
}
    
static ScriptState* getScriptStateForProtectedPtr(ScriptState* scriptState)
{
    switch (scriptState->scriptType()) {
        case JSScriptType:
        {
            JSC::ExecState* exec = static_cast<JSScriptState*>(scriptState)->execState()->lexicalGlobalObject()->globalExec();
            return JSScriptState::forExecState(exec);
        }
            
        case RBScriptType:
            return RBDOMBinding::globalScriptState(static_cast<RBScriptState*>(scriptState));
    }
}

ScriptStateProtectedPtr::ScriptStateProtectedPtr(ScriptState* scriptState)
    : m_scriptState(getScriptStateForProtectedPtr(scriptState))
{
}
    
ScriptStateProtectedPtr::~ScriptStateProtectedPtr()
{
}
    
DOMWindow* domWindowFromScriptState(ScriptState* state)
{
    return state->domWindow();
}

ScriptExecutionContext* scriptExecutionContextFromScriptState(ScriptState* state)
{
    return state->scriptExecutionContext();
}

bool evalEnabled(ScriptState* state)
{
    return state->evalEnabled();
}

void setEvalEnabled(ScriptState* state, bool set)
{
    state->setEvalEnabled(set);
}

ScriptState* mainWorldScriptState(Frame* frame, ScriptType type)
{
    switch (type) {
        case JSScriptType:
            return JSScriptState::mainWorldScriptState(frame);
        case RBScriptType:
            return RBScriptState::mainWorldScriptState(frame);
    }
}

} // namespace WebCore
