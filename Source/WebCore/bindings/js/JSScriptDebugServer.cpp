/*
 * Copyright (C) 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2010-2011 Google Inc. All rights reserved.
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

#include "config.h"

#if ENABLE(JAVASCRIPT_DEBUGGER)

#include "JSScriptDebugServer.h"

#include "ContentSearchUtils.h"
#include "Frame.h"
#include "JSJavaScriptCallFrame.h"
#include "JSScriptCallFrame.h"
#include "JSScriptState.h"
#include "JavaScriptCallFrame.h"
#include "ScriptBreakpoint.h"
#include "ScriptDebugListener.h"
#include "ScriptValue.h"
#include <debugger/DebuggerCallFrame.h>
#include <parser/SourceProvider.h>
#include <runtime/JSLock.h>
#include <wtf/MainThread.h>
#include <wtf/text/WTFString.h>

using namespace JSC;

namespace WebCore {

#define JS_CURRENT_CALL_FRAME static_cast<JSScriptCallFrame*>(m_currentCallFrame.get())

JSScriptDebugServer::JSScriptDebugServer()
    : ScriptDebugServer(JSScriptType)
{
}

JSScriptDebugServer::~JSScriptDebugServer()
{
}

void JSScriptDebugServer::attach(JSGlobalObject* globalObject)
{
    Debugger::attach(globalObject);
}

bool JSScriptDebugServer::evaluateBreakpointCondition(const String& condition) const
{
    JSValue exception;
    JSValue result = JS_CURRENT_CALL_FRAME->evaluate(condition, exception);
    if (exception) {
        // An erroneous condition counts as "false".
        return false;
    }
    return result.toBoolean(JS_CURRENT_CALL_FRAME->exec());
}

void JSScriptDebugServer::dispatchDidPause(ScriptDebugListener* listener)
{
    ASSERT(m_paused);
    JSGlobalObject* globalObject = JS_CURRENT_CALL_FRAME->scopeChain()->globalObject();
    JSC::ExecState* state = globalObject->globalExec();
    JSValue jsCallFrame;
    {
        if (JS_CURRENT_CALL_FRAME->isValid() && globalObject->inherits(&JSDOMGlobalObject::s_info)) {
            JSDOMGlobalObject* domGlobalObject = jsCast<JSDOMGlobalObject*>(globalObject);
            JSLockHolder lock(state);
            jsCallFrame = toJS(state, domGlobalObject, m_currentCallFrame.get());
        } else
            jsCallFrame = jsUndefined();
    }
    listener->didPause(JSScriptState::forExecState(state), ScriptValue(state->globalData(), jsCallFrame), ScriptValue());
}

void JSScriptDebugServer::dispatchDidParseSource(const ListenerSet& listeners, SourceProvider* sourceProvider, bool isContentScript)
{
    String sourceID = String::number(sourceProvider->asID());

    ScriptDebugListener::Script script;
    script.url = sourceProvider->url();
    script.source = sourceProvider->source();
    script.startLine = sourceProvider->startPosition().m_line.zeroBasedInt();
    script.startColumn = sourceProvider->startPosition().m_column.zeroBasedInt();
    script.isContentScript = isContentScript;

    setScriptSourceLinesAndColumns(script);

    Vector<ScriptDebugListener*> copy;
    copyToVector(listeners, copy);
    for (size_t i = 0; i < copy.size(); ++i)
        copy[i]->didParseSource(sourceID, script);
}

void JSScriptDebugServer::dispatchFailedToParseSource(const ListenerSet& listeners, SourceProvider* sourceProvider, int errorLine, const String& errorMessage)
{
    String url = sourceProvider->url();
    const String& data = sourceProvider->source();
    int firstLine = sourceProvider->startPosition().m_line.oneBasedInt();

    Vector<ScriptDebugListener*> copy;
    copyToVector(listeners, copy);
    for (size_t i = 0; i < copy.size(); ++i)
        copy[i]->failedToParseSource(url, data, firstLine, errorLine, errorMessage);
}

bool JSScriptDebugServer::isContentScript(ExecState* exec)
{
    return currentWorld(exec) != mainThreadNormalWorld();
}

void JSScriptDebugServer::detach(JSGlobalObject* globalObject)
{
    // If we're detaching from the currently executing global object, manually tear down our
    // stack, since we won't get further debugger callbacks to do so. Also, resume execution,
    // since there's no point in staying paused once a window closes.
    if (m_currentCallFrame && JS_CURRENT_CALL_FRAME->dynamicGlobalObject() == globalObject) {
        m_currentCallFrame = 0;
        m_pauseOnCallFrame = 0;
        continueProgram();
    }
    Debugger::detach(globalObject);
}

void JSScriptDebugServer::sourceParsed(ExecState* exec, SourceProvider* sourceProvider, int errorLine, const String& errorMessage)
{
    if (m_callingListeners)
        return;

    ListenerSet* listeners = getListenersForGlobalObject(exec->lexicalGlobalObject());
    if (!listeners)
        return;
    ASSERT(!listeners->isEmpty());

    m_callingListeners = true;

    bool isError = errorLine != -1;
    if (isError)
        dispatchFailedToParseSource(*listeners, sourceProvider, errorLine, errorMessage);
    else
        dispatchDidParseSource(*listeners, sourceProvider, isContentScript(exec));

    m_callingListeners = false;
}

void JSScriptDebugServer::createCallFrame(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
{
    TextPosition textPosition(OrdinalNumber::fromOneBasedInt(lineNumber), OrdinalNumber::fromZeroBasedInt(columnNumber));
    m_currentCallFrame = JSScriptCallFrame::create(debuggerCallFrame, m_currentCallFrame, sourceID, textPosition);
    if (m_lastExecutedSourceId != sourceID) {
        m_lastExecutedLine = -1;
        m_lastExecutedSourceId = sourceID;
    }
}

void JSScriptDebugServer::updateCallFrameAndPauseIfNeeded(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
{
    ASSERT(m_currentCallFrame);
    if (!m_currentCallFrame)
        return;

    TextPosition textPosition(OrdinalNumber::fromOneBasedInt(lineNumber), OrdinalNumber::fromZeroBasedInt(columnNumber));
    JS_CURRENT_CALL_FRAME->update(debuggerCallFrame, sourceID, textPosition);
    pauseIfNeeded();
}

void JSScriptDebugServer::callEvent(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
{
    if (!m_paused) {
        createCallFrame(debuggerCallFrame, sourceID, lineNumber, columnNumber);
        pauseIfNeeded();
    }
}

void JSScriptDebugServer::atStatement(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
{
    if (!m_paused)
        updateCallFrameAndPauseIfNeeded(debuggerCallFrame, sourceID, lineNumber, columnNumber);
}

void JSScriptDebugServer::returnEvent(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
{
    if (m_paused)
        return;

    updateCallFrameAndPauseIfNeeded(debuggerCallFrame, sourceID, lineNumber, columnNumber);

    // Treat stepping over a return statement like stepping out.
    if (!m_currentCallFrame)
        return;
    if (m_currentCallFrame == m_pauseOnCallFrame) {
        m_pauseOnCallFrame = m_currentCallFrame->caller();
        if (!m_currentCallFrame)
            return;
    }
    m_currentCallFrame = m_currentCallFrame->caller();
}

void JSScriptDebugServer::exception(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber, bool hasHandler)
{
    if (m_paused)
        return;

    if (m_pauseOnExceptionsState == PauseOnAllExceptions || (m_pauseOnExceptionsState == PauseOnUncaughtExceptions && !hasHandler))
        m_pauseOnNextStatement = true;

    updateCallFrameAndPauseIfNeeded(debuggerCallFrame, sourceID, lineNumber, columnNumber);
}

void JSScriptDebugServer::willExecuteProgram(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
{
    if (!m_paused) {
        createCallFrame(debuggerCallFrame, sourceID, lineNumber, columnNumber);
        pauseIfNeeded();
    }
}

void JSScriptDebugServer::didExecuteProgram(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
{
    if (m_paused)
        return;

    updateCallFrameAndPauseIfNeeded(debuggerCallFrame, sourceID, lineNumber, columnNumber);

    // Treat stepping over the end of a program like stepping out.
    if (m_currentCallFrame == m_pauseOnCallFrame)
        m_pauseOnCallFrame = m_currentCallFrame->caller();
    m_currentCallFrame = m_currentCallFrame->caller();
}

void JSScriptDebugServer::didReachBreakpoint(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
{
    if (m_paused)
        return;

    m_pauseOnNextStatement = true;
    updateCallFrameAndPauseIfNeeded(debuggerCallFrame, sourceID, lineNumber, columnNumber);
}

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)
