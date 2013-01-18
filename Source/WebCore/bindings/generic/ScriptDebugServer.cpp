/*
 * Copyright (C) 2008, 2009 Apple Inc. All rights reserved.
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

#include "config.h"

#if ENABLE(JAVASCRIPT_DEBUGGER)

#include "ScriptDebugServer.h"

#include "JavaScriptCallFrame.h"

namespace WebCore {

ScriptDebugServer::ScriptDebugServer(ScriptType type)
    : m_callingListeners(false)
    , m_pauseOnExceptionsState(DontPauseOnExceptions)
    , m_pauseOnNextStatement(false)
    , m_paused(false)
    , m_runningNestedMessageLoop(false)
    , m_doneProcessingDebuggerEvents(true)
    , m_breakpointsActivated(true)
    , m_pauseOnCallFrame(0)
    , m_lastExecutedLine(-1)
    , m_lastExecutedSourceId(-1)
    , m_scriptType(type)
    , m_recompileTimer(this, &ScriptDebugServer::recompileAllJSFunctions)
{
}

ScriptDebugServer::~ScriptDebugServer()
{
}

bool ScriptDebugServer::canSetScriptSource()
{
    return false;
}

bool ScriptDebugServer::setScriptSource(const String&, const String&, bool, String*, ScriptValue*, ScriptObject*)
{
    // FIXME(40300): implement this.
    return false;
}

void ScriptDebugServer::updateCallStack(ScriptValue*)
{
    // This method is used for restart frame feature that is not implemented yet.
    // FIXME(40300): implement this.
}

void ScriptDebugServer::compileScript(ScriptState*, const String&, const String&, String*, String*)
{
    // FIXME(89652): implement this.
}

void ScriptDebugServer::clearCompiledScripts()
{
    // FIXME(89652): implement this.
}

void ScriptDebugServer::runScript(ScriptState*, const String&, ScriptValue*, bool*, String*)
{
    // FIXME(89652): implement this.
}

String ScriptDebugServer::setBreakpoint(const String& sourceID, const ScriptBreakpoint& scriptBreakpoint, int* actualLineNumber, int* actualColumnNumber)
{
    intptr_t sourceIDValue = sourceID.toIntPtr();
    if (!sourceIDValue)
        return "";
    SourceIdToBreakpointsMap::iterator it = m_sourceIdToBreakpoints.find(sourceIDValue);
    if (it == m_sourceIdToBreakpoints.end())
        it = m_sourceIdToBreakpoints.set(sourceIDValue, LineToBreakpointMap()).iterator;
    LineToBreakpointMap::iterator breaksIt = it->value.find(scriptBreakpoint.lineNumber + 1);
    if (breaksIt == it->value.end())
        breaksIt = it->value.set(scriptBreakpoint.lineNumber + 1, BreakpointsInLine()).iterator;

    BreakpointsInLine& breaksVector = breaksIt->value;
    unsigned breaksCount = breaksVector.size();
    for (unsigned i = 0; i < breaksCount; i++) {
        if (breaksVector.at(i).columnNumber == scriptBreakpoint.columnNumber)
            return "";
    }
    breaksVector.append(scriptBreakpoint);

    *actualLineNumber = scriptBreakpoint.lineNumber;
    *actualColumnNumber = scriptBreakpoint.columnNumber;
    return sourceID + ":" + String::number(scriptBreakpoint.lineNumber) + ":" + String::number(scriptBreakpoint.columnNumber);
}

void ScriptDebugServer::removeBreakpoint(const String& breakpointId)
{
    Vector<String> tokens;
    breakpointId.split(":", tokens);
    if (tokens.size() != 3)
        return;
    bool success;
    intptr_t sourceIDValue = tokens[0].toIntPtr(&success);
    if (!success)
        return;
    unsigned lineNumber = tokens[1].toUInt(&success);
    if (!success)
        return;
    unsigned columnNumber = tokens[2].toUInt(&success);
    if (!success)
        return;

    SourceIdToBreakpointsMap::iterator it = m_sourceIdToBreakpoints.find(sourceIDValue);
    if (it == m_sourceIdToBreakpoints.end())
        return;
    LineToBreakpointMap::iterator breaksIt = it->value.find(lineNumber + 1);
    if (breaksIt == it->value.end())
        return;

    BreakpointsInLine& breaksVector = breaksIt->value;
    unsigned breaksCount = breaksVector.size();
    for (unsigned i = 0; i < breaksCount; i++) {
        if (breaksVector.at(i).columnNumber == static_cast<int>(columnNumber)) {
            breaksVector.remove(i);
            break;
        }
    }
}

bool ScriptDebugServer::hasBreakpoint(intptr_t sourceID, const TextPosition& position) const
{
    if (!m_breakpointsActivated)
        return false;

    SourceIdToBreakpointsMap::const_iterator it = m_sourceIdToBreakpoints.find(sourceID);
    if (it == m_sourceIdToBreakpoints.end())
        return false;

    int lineNumber = position.m_line.zeroBasedInt();
    int columnNumber = position.m_column.zeroBasedInt();
    if (lineNumber < 0 || columnNumber < 0)
        return false;

    LineToBreakpointMap::const_iterator breaksIt = it->value.find(lineNumber + 1);
    if (breaksIt == it->value.end())
        return false;

    bool hit = false;
    const BreakpointsInLine& breaksVector = breaksIt->value;
    unsigned breaksCount = breaksVector.size();
    unsigned i;
    for (i = 0; i < breaksCount; i++) {
        int breakLine = breaksVector.at(i).lineNumber;
        int breakColumn = breaksVector.at(i).columnNumber;
        // Since frontend truncates the indent, the first statement in a line must match the breakpoint (line,0).
        if ((lineNumber != m_lastExecutedLine && lineNumber == breakLine && !breakColumn)
            || (lineNumber == breakLine && columnNumber == breakColumn)) {
            hit = true;
            break;
        }
    }
    if (!hit)
        return false;

    // An empty condition counts as no condition which is equivalent to "true".
    if (breaksVector.at(i).condition.isEmpty())
        return true;

    return evaluateBreakpointCondition(breaksVector.at(i).condition);
}

void ScriptDebugServer::clearBreakpoints()
{
    m_sourceIdToBreakpoints.clear();
}

void ScriptDebugServer::setBreakpointsActivated(bool activated)
{
    m_breakpointsActivated = activated;
}

void ScriptDebugServer::setPauseOnExceptionsState(PauseOnExceptionsState pause)
{
    m_pauseOnExceptionsState = pause;
}

void ScriptDebugServer::setPauseOnNextStatement(bool pause)
{
    m_pauseOnNextStatement = pause;
}

void ScriptDebugServer::breakProgram()
{
    if (m_paused || !m_currentCallFrame)
        return;

    m_pauseOnNextStatement = true;
    pauseIfNeeded();
}

void ScriptDebugServer::continueProgram()
{
    if (!m_paused)
        return;

    m_pauseOnNextStatement = false;
    m_doneProcessingDebuggerEvents = true;
}

void ScriptDebugServer::stepIntoStatement()
{
    if (!m_paused)
        return;

    m_pauseOnNextStatement = true;
    m_doneProcessingDebuggerEvents = true;
}

void ScriptDebugServer::stepOverStatement()
{
    if (!m_paused)
        return;

    m_pauseOnCallFrame = m_currentCallFrame.get();
    m_doneProcessingDebuggerEvents = true;
}

void ScriptDebugServer::stepOutOfFunction()
{
    if (!m_paused)
        return;

    m_pauseOnCallFrame = m_currentCallFrame ? m_currentCallFrame->caller() : 0;
    m_doneProcessingDebuggerEvents = true;
}

void ScriptDebugServer::pauseIfNeeded()
{
    if (m_paused)
        return;
 
    if (!getListenersForCurrentFrame())
        return;

    bool pauseNow = m_pauseOnNextStatement;
    pauseNow |= (m_pauseOnCallFrame == m_currentCallFrame);
    pauseNow |= hasBreakpoint(m_currentCallFrame->sourceID(), m_currentCallFrame->position());
    m_lastExecutedLine = m_currentCallFrame->position().m_line.zeroBasedInt();
    if (!pauseNow)
        return;

    m_pauseOnCallFrame = 0;
    m_pauseOnNextStatement = false;
    m_paused = true;

    dispatchFunctionToListeners(&ScriptDebugServer::dispatchDidPause);
    didPause();

    TimerBase::fireTimersInNestedEventLoop();

    m_runningNestedMessageLoop = true;
    m_doneProcessingDebuggerEvents = false;
    runEventLoopWhilePaused();
    m_runningNestedMessageLoop = false;

    didContinue();
    dispatchFunctionToListeners(&ScriptDebugServer::dispatchDidContinue);

    m_paused = false;
}

void ScriptDebugServer::dispatchDidContinue(ScriptDebugListener* listener)
{
    listener->didContinue();
}

void ScriptDebugServer::dispatchFunctionToListeners(const ListenerSet& listeners, JavaScriptExecutionCallback callback)
{
    Vector<ScriptDebugListener*> copy;
    copyToVector(listeners, copy);
    for (size_t i = 0; i < copy.size(); ++i)
        (this->*callback)(copy[i]);
}

void ScriptDebugServer::dispatchFunctionToListeners(JavaScriptExecutionCallback callback)
{
    if (m_callingListeners)
        return;

    m_callingListeners = true;

    if (ListenerSet* listeners = getListenersForCurrentFrame()) {
        ASSERT(!listeners->isEmpty());
        dispatchFunctionToListeners(*listeners, callback);
    }

    m_callingListeners = false;
}

void ScriptDebugServer::recompileAllJSFunctionsSoon()
{
    m_recompileTimer.startOneShot(0);
}

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)
