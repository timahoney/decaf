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

#ifndef RBScriptState_h
#define RBScriptState_h

#include "ScriptState.h"
#include <Ruby/ruby.h>

namespace WebCore {
class DOMWindow;
class Frame;
class ScriptExecutionContext;

class RBScriptState : public ScriptState {
public:
    // Returns the script state according to the current Ruby call frame.
    static RBScriptState* current();
    
    static RBScriptState* forBinding(VALUE binding);

    VALUE binding() const;
    
    virtual bool hadException();
    virtual DOMWindow* domWindow() const;
    virtual ScriptExecutionContext* scriptExecutionContext() const;
    virtual bool evalEnabled() const;
    virtual void setEvalEnabled(bool);
    
    static RBScriptState* mainWorldScriptState(Frame*);
    static PassRefPtr<ScriptCallStack> createScriptCallStack(size_t maxStackSize, bool emptyStackIsAllowed);
    virtual PassRefPtr<ScriptCallStack> createScriptCallStack(size_t maxStackSize);
    virtual PassRefPtr<ScriptCallStack> createScriptCallStackForConsole();

private:
    RBScriptState(VALUE binding, VALUE window);
    virtual ~RBScriptState();

    VALUE m_binding;
    VALUE m_window;
    bool m_evalEnabled;
};

} // namespace WebCore

#endif // RBScriptState_h
