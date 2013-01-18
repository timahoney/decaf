/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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
#include "BindingState.h"

#include "JSDOMWindowCustom.h"
#include "JSScriptState.h"
#include "RBDOMWindow.h"
#include "RBScriptState.h"

namespace WebCore {

static DOMWindow* rbActiveDOMWindow(RBScriptState* state)
{
    return state->domWindow();
}

static DOMWindow* rbFirstDOMWindow(RBScriptState* state)
{
    DOMWindow* window = state->domWindow();
    DOMWindow* nextWindow = window->parent();
    do {
        window = nextWindow;
        nextWindow = window->parent();
    } while (nextWindow != window);
    
    return window;
}
    
DOMWindow* activeDOMWindow(ScriptState* state)
{
    switch (state->scriptType()) {
    case JSScriptType:
        return activeDOMWindow(static_cast<JSScriptState*>(state)->execState());
    case RBScriptType:
        return rbActiveDOMWindow(static_cast<RBScriptState*>(state));
    }
}

DOMWindow* firstDOMWindow(ScriptState* state)
{
    switch (state->scriptType()) {
    case JSScriptType:
        return firstDOMWindow(static_cast<JSScriptState*>(state)->execState());
    case RBScriptType:
        return rbFirstDOMWindow(static_cast<RBScriptState*>(state));
    }
}

DOMWindow* activeDOMWindow(JSC::ExecState* exec)
{
    return asJSDOMWindow(exec->lexicalGlobalObject())->impl();
}

DOMWindow* firstDOMWindow(JSC::ExecState* exec)
{
    return asJSDOMWindow(exec->dynamicGlobalObject())->impl();
}

}
