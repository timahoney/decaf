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

#ifndef ScriptState_h
#define ScriptState_h

#include "ScriptType.h"
#include <wtf/Noncopyable.h>
#include <wtf/RefPtr.h>

namespace JSC {
class ExecState;
}

namespace WebCore {
class DOMWindow;
class DOMWrapperWorld;
class Frame;
class Node;
class Page;
class ScriptArguments;
class ScriptCallStack;
class ScriptExecutionContext;
class WorkerContext;
    
class ScriptState {
public:
    virtual ~ScriptState();
    
    ScriptType scriptType() const { return m_type; }
    
    virtual bool hadException() = 0;
    virtual ScriptExecutionContext* scriptExecutionContext() const = 0;
    virtual DOMWindow* domWindow() const = 0;
    virtual bool evalEnabled() const = 0;
    virtual void setEvalEnabled(bool) = 0;

    virtual PassRefPtr<ScriptCallStack> createScriptCallStack(size_t maxStackSize) = 0;
    virtual PassRefPtr<ScriptCallStack> createScriptCallStackForConsole() = 0;

protected:
    ScriptState(ScriptType type);
    
private:
    ScriptType m_type;
};

class ScriptStateProtectedPtr {
    WTF_MAKE_NONCOPYABLE(ScriptStateProtectedPtr);
public:
    explicit ScriptStateProtectedPtr(ScriptState* scriptState);
    ~ScriptStateProtectedPtr();
    ScriptState* get() const { return m_scriptState; }

private:
    ScriptState* m_scriptState;
};

// FIXME: Remove these in favor of the ScriptState member functions.
DOMWindow* domWindowFromScriptState(ScriptState*);
ScriptExecutionContext* scriptExecutionContextFromScriptState(ScriptState*);
bool evalEnabled(ScriptState*);
void setEvalEnabled(ScriptState*, bool);

ScriptState* mainWorldScriptState(Frame*, ScriptType type);

ScriptState* scriptStateFromNode(DOMWrapperWorld*, Node*);
ScriptState* scriptStateFromPage(DOMWrapperWorld*, Page*);

#if ENABLE(WORKERS)
ScriptState* scriptStateFromWorkerContext(WorkerContext*);
#endif
    
} // namespace WebCore

#endif
