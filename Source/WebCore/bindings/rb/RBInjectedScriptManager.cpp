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

#if ENABLE(INSPECTOR)

#include "RBInjectedScriptManager.h"

#include "BindingSecurity.h"
#include "ExceptionCode.h"
#include "RBCallHelpers.h"
#include "RBConverters.h"
#include "RBDOMWindow.h"
#include "RBInjectedScriptHost.h"
#include "RBScriptState.h"
#include "RBScriptValue.h"
#include "ScriptObject.h"

namespace WebCore {

using namespace RB;

ScriptObject rbCreateInjectedScript(const String& source, ScriptState* state, int id, InjectedScriptHost* host)
{
    VALUE injectedScriptSource = rb_str_new2(source.utf8().data());
    VALUE binding = static_cast<RBScriptState*>(state)->binding();
    VALUE rb_cInjectedScript = callFunction(binding, "eval", injectedScriptSource, rb_str_new2("InjectedScriptSource.rb"));

    // FIXME: We shouldn't need to use @inspected_window.
    // It's only used in RBInjectedScriptHost::evaluate.
    // See the comment there and try to remove this instance variable.
    VALUE hostRB = toRB(host);
    VALUE windowRB = toRB(state->domWindow());
    rb_iv_set(hostRB, "@inspected_window", windowRB);
    VALUE injectedScript = rb_funcall(rb_cInjectedScript, rb_intern("new"), 3, hostRB, windowRB, toRB(id));

    if (NIL_P(injectedScript))
        return ScriptObject();
    
    return ScriptObject(state, RBScriptValue::create(injectedScript));
}

bool rbCanAccessInspectedWindow(ScriptState* state)
{
    RBScriptState* scriptState = static_cast<RBScriptState*>(state);
    DOMWindow* inspectedWindow = scriptState->domWindow();
    if (!inspectedWindow)
        return false;
    return BindingSecurity::shouldAllowAccessToDOMWindow(scriptState, inspectedWindow, DoNotReportSecurityError);
}

} // namespace WebCore

#endif // ENABLE(INSPECTOR)
