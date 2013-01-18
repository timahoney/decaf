/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2010-2011 Google Inc. All rights reserved.
 * Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ScriptDebugServer_h
#define ScriptDebugServer_h

#if ENABLE(JAVASCRIPT_DEBUGGER)

#include "ScriptBreakpoint.h"
#include "ScriptDebugListener.h"
#include "ScriptType.h"
#include "Timer.h"
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>
#include <wtf/text/TextPosition.h>
#include <wtf/text/WTFString.h>

namespace JSC {
class JSGlobalObject;
}

namespace WebCore {

class JavaScriptCallFrame;
class ScriptDebugListener;
class ScriptObject;
class ScriptValue;

class ScriptDebugServer {
    WTF_MAKE_NONCOPYABLE(ScriptDebugServer); WTF_MAKE_FAST_ALLOCATED;
public:
    virtual String setBreakpoint(const String& sourceID, const ScriptBreakpoint&, int* actualLineNumber, int* actualColumnNumber);
    virtual void removeBreakpoint(const String& breakpointId);
    virtual void clearBreakpoints();
    virtual void setBreakpointsActivated(bool activated);
    void activateBreakpoints() { setBreakpointsActivated(true); }
    void deactivateBreakpoints() { setBreakpointsActivated(false); }

    enum PauseOnExceptionsState {
        DontPauseOnExceptions,
        PauseOnAllExceptions,
        PauseOnUncaughtExceptions
    };
    virtual PauseOnExceptionsState pauseOnExceptionsState() const { return m_pauseOnExceptionsState; }
    virtual void setPauseOnExceptionsState(PauseOnExceptionsState);
    
    virtual void runEventLoopWhilePaused() = 0;
    
    // FIXME: Remove this.
    // We need this currently because of problems when casting virtual pointers in JSScriptController::attachDebugger.
    // Fix that situation, and we won't need this pure virtual function.
    virtual void attach(JSC::JSGlobalObject*) { };

    virtual void setPauseOnNextStatement(bool pause);
    virtual void breakProgram();
    virtual void continueProgram();
    virtual void stepIntoStatement();
    virtual void stepOverStatement();
    virtual void stepOutOfFunction();

    bool canSetScriptSource();
    bool setScriptSource(const String& sourceID, const String& newContent, bool preview, String* error, ScriptValue* newCallFrames, ScriptObject* result);
    void updateCallStack(ScriptValue* callFrame);

    bool causesRecompilation() { return true; }
    bool supportsSeparateScriptCompilationAndExecution() { return false; }

    void setScriptPreprocessor(const String&)
    {
        // FIXME(webkit.org/b/82203): Implement preprocessor.
    }

    bool runningNestedMessageLoop() { return m_runningNestedMessageLoop; }
    virtual bool isPaused() { return m_paused; }

    void compileScript(ScriptState*, const String& expression, const String& sourceURL, String* scriptId, String* exceptionMessage);
    void clearCompiledScripts();
    void runScript(ScriptState*, const String& scriptId, ScriptValue* result, bool* wasThrown, String* exceptionMessage);
    
    // FIXME: These should have non-JS-specific names.
    void recompileAllJSFunctionsSoon();
    virtual void recompileAllJSFunctions(Timer<ScriptDebugServer>*) = 0;
    
    virtual void didClearMainFrameWindowObject() { }

    class Task {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        virtual ~Task() { }
        virtual void run() = 0;
    };
    
    ScriptType scriptType() const { return m_scriptType; }

protected:
    typedef HashSet<ScriptDebugListener*> ListenerSet;
    typedef void (ScriptDebugServer::*JavaScriptExecutionCallback)(ScriptDebugListener*);

    ScriptDebugServer(ScriptType);
    virtual ~ScriptDebugServer();

    virtual ListenerSet* getListenersForCurrentFrame() = 0;
    virtual void didPause() = 0;
    virtual void didContinue() = 0;

    bool hasBreakpoint(intptr_t sourceID, const TextPosition&) const;
    virtual bool evaluateBreakpointCondition(const String& condition) const = 0;

    void dispatchFunctionToListeners(JavaScriptExecutionCallback);
    void dispatchFunctionToListeners(const ListenerSet& listeners, JavaScriptExecutionCallback callback);
    virtual void dispatchDidPause(ScriptDebugListener*) = 0;
    virtual void dispatchDidContinue(ScriptDebugListener*);

    void pauseIfNeeded();

    typedef Vector<ScriptBreakpoint> BreakpointsInLine;
    typedef HashMap<long, BreakpointsInLine> LineToBreakpointMap;
    typedef HashMap<intptr_t, LineToBreakpointMap> SourceIdToBreakpointsMap;

    bool m_callingListeners;
    PauseOnExceptionsState m_pauseOnExceptionsState;
    bool m_pauseOnNextStatement;
    bool m_paused;
    bool m_runningNestedMessageLoop;
    bool m_doneProcessingDebuggerEvents;
    bool m_breakpointsActivated;
    JavaScriptCallFrame* m_pauseOnCallFrame;
    RefPtr<JavaScriptCallFrame> m_currentCallFrame;
    SourceIdToBreakpointsMap m_sourceIdToBreakpoints;

    int m_lastExecutedLine;
    intptr_t m_lastExecutedSourceId;
    
private:
    ScriptType m_scriptType;
    Timer<ScriptDebugServer> m_recompileTimer;
};

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)

#endif // ScriptDebugServer_h
