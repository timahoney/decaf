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
#include "RBScriptState.h"

#include "Frame.h"
#include "RBConverters.h"
#include "RBDOMBinding.h"
#include "RBDOMWindow.h"
#include "RBObject.h"
#include "RBScriptCallStackFactory.h"
#include "ScriptArguments.h"
#include "WorkerContext.h"

using namespace RB;

namespace WebCore {

RBScriptState::RBContextToGlobalStateMap* RBScriptState::s_contextGlobalStates;

RBScriptState::RBScriptState(VALUE binding)
    : ScriptState(RBScriptType)
    , m_binding(binding)
    , m_evalEnabled(true)
{
    rb_gc_register_address(&m_binding);
}

RBScriptState::~RBScriptState()
{
    rb_gc_unregister_address(&m_binding);
}

RBScriptState* RBScriptState::current()
{
    return forBinding(rb_binding_new());
}

RBScriptState* RBScriptState::forBinding(VALUE binding)
{
    // FIXME: Do we really need this? Can't we just use 'new'?
    return new RBScriptState(binding);
}
    
bool RBScriptState::hadException()
{
    // FIXME: How do we tell if a script had an exception within it's own binding context?
    return !NIL_P(rb_errinfo());
}

DOMWindow* RBScriptState::domWindow() const
{
    ScriptExecutionContext* context = scriptExecutionContext();
    if (!context || !context->isDocument())
        return 0;

    return static_cast<Document*>(context)->domWindow();
}

VALUE RBScriptState::binding() const
{
    return m_binding;
}

ScriptExecutionContext* RBScriptState::scriptExecutionContext() const
{
    return contextFromBinding(m_binding);
}

bool RBScriptState::evalEnabled() const
{
    return m_evalEnabled;
}

void RBScriptState::setEvalEnabled(bool enabled)
{
    m_evalEnabled = enabled;
}

ScriptState* RBScriptState::globalScriptState(ScriptExecutionContext* context)
{
    if (!s_contextGlobalStates)
        s_contextGlobalStates = new RBContextToGlobalStateMap();
    
    // FIXME: Delete these ScriptStates when the context is finished.
    RBScriptState* globalState = s_contextGlobalStates->get(context);
    if (!globalState) {
        VALUE binding = bindingFromContext(context);
        globalState = new RBScriptState(binding);
        s_contextGlobalStates->set(context, globalState);
    }
    
    return globalState;
}
    
ScriptState* RBScriptState::mainWorldScriptState(Frame* frame)
{
    return globalScriptState(frame->document());
}

ScriptState* RBScriptState::scriptStateFromWorkerContext(WorkerContext* workerContext)
{
    return globalScriptState(workerContext);
}

PassRefPtr<ScriptCallStack> RBScriptState::createScriptCallStack(size_t maxStackSize, bool emptyStackIsAllowed)
{
    return RBScriptCallStackFactory::createScriptCallStack(maxStackSize, emptyStackIsAllowed);
}

PassRefPtr<ScriptCallStack> RBScriptState::createScriptCallStack(size_t maxStackSize)
{
    return RBScriptCallStackFactory::createScriptCallStack(this, maxStackSize);
}

PassRefPtr<ScriptCallStack> RBScriptState::createScriptCallStackForConsole()
{
    return RBScriptCallStackFactory::createScriptCallStackForConsole(this);
}

} // namespace WebCore
