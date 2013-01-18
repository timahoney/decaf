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

namespace WebCore {

RBScriptState::RBScriptState(VALUE binding, VALUE window)
    : ScriptState(RBScriptType)
    , m_binding(binding)
    , m_window(window)
    , m_evalEnabled(true)
{
    rb_gc_register_address(&m_binding);
    rb_gc_register_address(&m_window);
}

RBScriptState::~RBScriptState()
{
    // FIXME: Should we put this back in?
    // rb_gc_unregister_address(&m_binding);
    // rb_gc_unregister_address(&m_window);
}

RBScriptState* RBScriptState::current()
{
    return forBinding(rb_binding_new());
}

RBScriptState* RBScriptState::forBinding(VALUE binding)
{
    VALUE window = rb_funcall(binding, rb_intern("eval"), 1, rb_str_new2("$window"));
    return new RBScriptState(binding, window);
}
    
bool RBScriptState::hadException()
{
    // FIXME: How do we tell if a script had an exception within it's own binding context?
    return !NIL_P(rb_errinfo());
}

DOMWindow* RBScriptState::domWindow() const
{
    DOMWindow* window = impl<DOMWindow>(m_window);
    return window;
}

VALUE RBScriptState::binding() const
{
    return m_binding;
}

ScriptExecutionContext* RBScriptState::scriptExecutionContext() const
{
    return domWindow()->scriptExecutionContext();
}

bool RBScriptState::evalEnabled() const
{
    return m_evalEnabled;
}

void RBScriptState::setEvalEnabled(bool enabled)
{
    m_evalEnabled = enabled;
}
    
RBScriptState* RBScriptState::mainWorldScriptState(Frame* frame)
{
    DOMWindow* window = frame->document()->domWindow();
    VALUE windowRB = toRB(window);
    VALUE binding = RBDOMBinding::bindingFromWindow(window);

    // FIXME: This will get leaked...I think. V8 just returns a new one, can we as well?
    return new RBScriptState(binding, windowRB);
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
