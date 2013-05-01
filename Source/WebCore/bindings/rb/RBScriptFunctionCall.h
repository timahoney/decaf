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

#ifndef RBScriptFunctionCall_h
#define RBScriptFunctionCall_h

#include "RBConverters.h"
#include "ScriptFunctionCall.h"
#include <Ruby/ruby.h>

namespace WebCore {
class RBScriptCallArgumentHandler : public virtual ScriptCallArgumentHandlerDelegate {
public:
    virtual void appendArgument(const ScriptObject&);
    virtual void appendArgument(const ScriptValue&);
    virtual void appendArgument(const char* argument) { appendRBArgument(toRB(argument)); }
    virtual void appendArgument(const String& argument) { appendRBArgument(toRB(argument)); }
    virtual void appendArgument(long argument) { appendRBArgument(toRB(argument)); }
    virtual void appendArgument(long long argument) { appendRBArgument(toRB(argument)); }
    virtual void appendArgument(unsigned int argument) { appendRBArgument(toRB(argument)); }
    virtual void appendArgument(unsigned long argument) { appendRBArgument(toRB(argument)); }
    virtual void appendArgument(int argument) { appendRBArgument(toRB(argument)); }
    virtual void appendArgument(bool argument) { appendRBArgument(toRB(argument)); }
    void appendRBArgument(VALUE argument);
    
    virtual ~RBScriptCallArgumentHandler();

protected:
    RBScriptCallArgumentHandler(ScriptState* state);

    Vector<ScriptValue> m_arguments;
};

class RBScriptFunctionCall : public RBScriptCallArgumentHandler, public ScriptFunctionCallDelegate {
public:
    static PassRefPtr<RBScriptFunctionCall> create(const ScriptObject& thisObject, const String& name)
    {
        return adoptRef(new RBScriptFunctionCall(thisObject, name));
    }

    virtual ScriptValue call(bool& hadException, bool reportExceptions = true);
    virtual ScriptObject construct(bool& hadException, bool reportExceptions = true);
    
    virtual ~RBScriptFunctionCall() { }

private:
    RBScriptFunctionCall(const ScriptObject& thisObject, const String& name)
    : ScriptCallArgumentHandlerDelegate(thisObject.scriptState())
    , RBScriptCallArgumentHandler(thisObject.scriptState())
    , ScriptFunctionCallDelegate(thisObject, name)
    {
    }
};

class RBScriptCallback : public RBScriptCallArgumentHandler, public ScriptCallbackDelegate {
public:
    static PassRefPtr<RBScriptCallback> create(ScriptState* state, const ScriptValue& function)
    {
        return adoptRef(new RBScriptCallback(state, function));
    }

    virtual ScriptValue call();
    
    virtual ~RBScriptCallback() { }

private:
    RBScriptCallback(ScriptState* state, const ScriptValue& function)
    : ScriptCallArgumentHandlerDelegate(state)
    , RBScriptCallArgumentHandler(state)
    , ScriptCallbackDelegate(state, function)
    {
    }
};

} // namespace WebCore

#endif // RBScriptFunctionCall
