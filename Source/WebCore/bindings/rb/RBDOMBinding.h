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

#ifndef RBDOMBinding_h
#define RBDOMBinding_h

#include "RBScriptState.h"
#include "ScriptExecutionContext.h"
#include <Ruby/ruby.h>

namespace WebCore {

class CachedScript;
class DOMWindow;

// FIXME: Use RB namespace instead of RBDOMBinding class.

class RBDOMBinding {
public:
    static void reportException(ScriptExecutionContext*, VALUE exception, CachedScript* = 0);
    static void reportCurrentException(RBScriptState*, CachedScript* = 0);

    // Returns the window for the current Ruby VM state.
    static DOMWindow* currentWindow();
    static VALUE currentWindowRB();

    static intptr_t sourceIDFromFileName(const char* fileName);
    
    // Finds the global script state for another state.
    static RBScriptState* globalScriptState(RBScriptState*);
};

namespace RB {
    ScriptExecutionContext* currentContext();

    // Gets the binding for execution.
    // Always evaluate any code from the user in this binding.
    // If you use a different binding than this one, 
    // then it will likely not have any of the previously user-defined code.
    VALUE bindingFromContext(ScriptExecutionContext* context);
    ScriptExecutionContext* contextFromBinding(VALUE binding);
} // namespace RB

} // namespace WebCore

#endif // RBDOMBinding_h
