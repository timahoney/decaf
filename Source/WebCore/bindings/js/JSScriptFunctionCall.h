/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
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

#ifndef JSScriptFunctionCall_h
#define JSScriptFunctionCall_h

#include "ScriptFunctionCall.h"
#include "JSScriptState.h"
#include <runtime/ArgList.h>

namespace JSC {
class JSValue;
}

namespace WebCore {
class JSScriptCallArgumentHandler : public virtual ScriptCallArgumentHandlerDelegate {
public:
    virtual void appendArgument(const ScriptObject&);
    virtual void appendArgument(const ScriptValue&);
    virtual void appendArgument(const char*);
    virtual void appendArgument(const String&);
    virtual void appendArgument(long);
    virtual void appendArgument(long long);
    virtual void appendArgument(unsigned int);
    virtual void appendArgument(unsigned long);
    virtual void appendArgument(int);
    virtual void appendArgument(bool);
    void appendArgument(JSC::JSValue value) { m_arguments.append(value); }
    
    virtual ~JSScriptCallArgumentHandler() { }

protected:
    JSScriptCallArgumentHandler(ScriptState* state)
    : ScriptCallArgumentHandlerDelegate(state)
    , m_exec(static_cast<JSScriptState*>(state)->execState())
    {
    }

    Vector<JSC::JSValue> m_arguments;
    JSC::ExecState* m_exec;
};

class JSScriptFunctionCall : public virtual ScriptFunctionCallDelegate, public virtual JSScriptCallArgumentHandler {
public:
    static PassRefPtr<JSScriptFunctionCall> create(const ScriptObject& thisObject, const String& name)
    {
        return adoptRef(new JSScriptFunctionCall(thisObject, name));
    }

    virtual ScriptValue call(bool& hadException, bool reportExceptions = true);
    virtual ScriptObject construct(bool& hadException, bool reportExceptions = true);
    
    virtual ~JSScriptFunctionCall() { }

private:
    JSScriptFunctionCall(const ScriptObject& thisObject, const String& name)
    : ScriptCallArgumentHandlerDelegate(thisObject.scriptState())
    , ScriptFunctionCallDelegate(thisObject, name)
    , JSScriptCallArgumentHandler(thisObject.scriptState())
    {
    }
};

class JSScriptCallback : public virtual ScriptCallbackDelegate, public virtual JSScriptCallArgumentHandler {
public:
    static PassRefPtr<JSScriptCallback> create(ScriptState* state, const ScriptValue& function)
    {
        return adoptRef(new JSScriptCallback(state, function));
    }

    virtual ScriptValue call();
    
    virtual ~JSScriptCallback() { }

private:
    JSScriptCallback(ScriptState* state, const ScriptValue& function)
    : ScriptCallArgumentHandlerDelegate(state)
    , ScriptCallbackDelegate(state, function)
    , JSScriptCallArgumentHandler(state)
    {
    }
};

} // namespace WebCore

#endif // JSScriptFunctionCall
