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

#ifndef ScriptFunctionCall_h
#define ScriptFunctionCall_h

#include "ScriptObject.h"
#include "ScriptState.h"
#include <wtf/text/WTFString.h>

namespace WebCore {
class ScriptValue;
class ScriptCallArgumentHandlerDelegate;
class ScriptFunctionCallDelegate;
class ScriptCallbackDelegate;

class ScriptCallArgumentHandler {
public:
    void appendArgument(const ScriptObject&);
    void appendArgument(const ScriptValue&);
    void appendArgument(const char*);
    void appendArgument(const String&);
    void appendArgument(long);
    void appendArgument(long long);
    void appendArgument(unsigned int);
    void appendArgument(unsigned long);
    void appendArgument(int);
    void appendArgument(bool);
    
    virtual ~ScriptCallArgumentHandler();

protected:
    ScriptCallArgumentHandler(ScriptState*);
    
    virtual ScriptCallArgumentHandlerDelegate* argumentDelegate() const = 0;

private:
    ScriptState* m_state;

    // Prevent heap alloc.
    void* operator new(size_t) { ASSERT_NOT_REACHED(); return reinterpret_cast<void*>(0xbadbeef); }
    void* operator new[](size_t) { ASSERT_NOT_REACHED(); return reinterpret_cast<void*>(0xbadbeef); }
};

class ScriptFunctionCall : public ScriptCallArgumentHandler {
public:
    ScriptFunctionCall(const ScriptObject& thisObject, const String& name);
    
    ScriptValue call(bool& hadException, bool reportExceptions = true);
    ScriptValue call();
    ScriptObject construct(bool& hadException, bool reportExceptions = true);
    
private:
    ScriptFunctionCall(PassRefPtr<ScriptFunctionCallDelegate> delegate);
    
    virtual ScriptCallArgumentHandlerDelegate* argumentDelegate() const;
    
    RefPtr<ScriptFunctionCallDelegate> m_delegate;
};

class ScriptCallback : public ScriptCallArgumentHandler {
public:
    ScriptCallback(ScriptState* state, const ScriptValue& function);
    
    ScriptValue call();

private:
    ScriptCallback(PassRefPtr<ScriptCallbackDelegate> delegate);
    
    virtual ScriptCallArgumentHandlerDelegate* argumentDelegate() const;
    
    RefPtr<ScriptCallbackDelegate> m_delegate;
};

// These delegates provide the language-specific functionality 
// for ScriptFunctionCall and ScriptCallback. 
// Each language should create a subclass of each of them, 
// then create the correct delegate in create(...) methods 
// of ScriptFunctionCallDelegate, and ScriptCallbackDelegate.

class ScriptCallArgumentHandlerDelegate : public RefCounted<ScriptCallArgumentHandlerDelegate> {
    
    friend class ScriptFunctionCall;
    friend class ScriptCallback;
    
public:
    virtual void appendArgument(const ScriptObject&) = 0;
    virtual void appendArgument(const ScriptValue&) = 0;
    virtual void appendArgument(const char*) = 0;
    virtual void appendArgument(const String&) = 0;
    virtual void appendArgument(long) = 0;
    virtual void appendArgument(long long) = 0;
    virtual void appendArgument(unsigned int) = 0;
    virtual void appendArgument(unsigned long) = 0;
    virtual void appendArgument(int) = 0;
    virtual void appendArgument(bool) = 0;
    
    virtual ~ScriptCallArgumentHandlerDelegate();

protected:
    ScriptCallArgumentHandlerDelegate(ScriptState* state);

    ScriptState* m_state;
};

class ScriptFunctionCallDelegate : public virtual ScriptCallArgumentHandlerDelegate {
public:
    // Creates a function call delegate of the correct type for the ScriptObject.
    static PassRefPtr<ScriptFunctionCallDelegate> create(const ScriptObject& thisObject, const String& name);
    
    virtual ScriptValue call(bool& hadException, bool reportExceptions = true) = 0;
    virtual ScriptObject construct(bool& hadException, bool reportExceptions = true) = 0;
    
    virtual ~ScriptFunctionCallDelegate();

protected:
    ScriptFunctionCallDelegate(const ScriptObject& thisObject, const String& name);

    ScriptObject m_thisObject;
    String m_name;
};

class ScriptCallbackDelegate : public virtual ScriptCallArgumentHandlerDelegate {
public:
    // Creates a callback delegate of the correct type for the ScriptState.
    static PassRefPtr<ScriptCallbackDelegate> create(ScriptState*, const ScriptValue&);
    
    virtual ScriptValue call() = 0;
    
    virtual ~ScriptCallbackDelegate();

protected:
    ScriptCallbackDelegate(ScriptState* state, const ScriptValue& function);

    ScriptValue m_function;
};

} // namespace WebCore

#endif // ScriptFunctionCall
