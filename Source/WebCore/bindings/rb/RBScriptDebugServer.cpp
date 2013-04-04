/*
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

#include "RBScriptDebugServer.h"

#include "RBDOMBinding.h"
#include "RBJavaScriptCallFrame.h"
#include "RBScriptCallFrame.h"
#include "RBScriptState.h"
#include "RBScriptValue.h"
#include <wtf/text/CString.h>
#include <wtf/HashSet.h>
#include <wtf/StringHasher.h>

namespace WebCore {

RBScriptDebugServer::RBScriptDebugServer()
    : ScriptDebugServer(RBScriptType)
    , m_doProcessEvents(true)
{
}

RBScriptDebugServer::~RBScriptDebugServer()
{
}

static bool isValidUrl(const String& filename)
{
    return filename.contains("://");
}

RBScriptCallFrame* RBScriptDebugServer::currentCallFrame() const
{
    return static_cast<RBScriptCallFrame*>(m_currentCallFrame.get());
}

void RBScriptDebugServer::createCurrentCallFrame()
{
    m_doProcessEvents = false;
    
    TextPosition textPosition(OrdinalNumber::fromOneBasedInt(rb_sourceline()), OrdinalNumber::fromZeroBasedInt(0));
    VALUE binding = rb_binding_new();
    intptr_t sourceID = RBDOMBinding::sourceIDFromFileName(rb_sourcefile());
    m_currentCallFrame = RBScriptCallFrame::create(binding, m_currentCallFrame, sourceID, textPosition);
    if (m_lastExecutedSourceId != sourceID) {
        m_lastExecutedLine = -1;
        m_lastExecutedSourceId = sourceID;
    }
    
    m_doProcessEvents = true;
}

void RBScriptDebugServer::updateCurrentCallFrame()
{
    if (!m_currentCallFrame) {
        createCurrentCallFrame();
        return;
    }
    
    TextPosition textPosition(OrdinalNumber::fromOneBasedInt(rb_sourceline()), OrdinalNumber::fromZeroBasedInt(0));
    m_doProcessEvents = false;
    int sourceID = RBDOMBinding::sourceIDFromFileName(rb_sourcefile());
    VALUE binding = rb_binding_new();
    currentCallFrame()->update(binding, sourceID, textPosition);
    m_doProcessEvents = true;
}

void RBScriptDebugServer::processEventHook(rb_event_flag_t event, VALUE data, VALUE self, ID mid, VALUE klass)
{
    // FIXME: At some point, we should switch over to the new TracePoint API.
    
    UNUSED_PARAM(self);
    UNUSED_PARAM(klass);
    UNUSED_PARAM(data);
    UNUSED_PARAM(mid);

    if (!m_doProcessEvents)
        return;

    // If we don't have a filename, then don't do anything here.
    // rb_sourcefile might return '1' instead of '0'
    const char* fileName = rb_sourcefile();
    if (!fileName || reinterpret_cast<intptr_t>(fileName) == 1)
        return;
    intptr_t sourceID = RBDOMBinding::sourceIDFromFileName(fileName);

    if (!m_parsedScriptIds.contains(sourceID) && isValidUrl(fileName)) {
        if (!m_currentCallFrame)
            createCurrentCallFrame();
        
        m_parsedScriptIds.add(sourceID);
        ListenerSet* listeners = getListenersForCurrentFrame();
        if (!listeners)
            return;
        dispatchDidParseSource(*listeners, rb_sourcefile());
    }

    switch (event) {
    case RUBY_EVENT_LINE: {
        // Sometimes, RUBY_EVENT_LINE will fire multiple times for the same line.
        // Make sure we don't pause twice on the same line.
        // Remember that m_lastExecutedLine is zero-based and rb_sourceline() is one-based.
        if (m_lastExecutedLine == rb_sourceline() - 1)
            return;
        updateCurrentCallFrame();
        pauseIfNeeded();
        break;
    }

    case RUBY_EVENT_CALL: {
        createCurrentCallFrame();
        pauseIfNeeded();
        break;
    }

    case RUBY_EVENT_RETURN: {
        updateCurrentCallFrame();
        pauseIfNeeded();
        if (!m_currentCallFrame)
            return;

        // Treat stepping over a return statement like stepping out.
        if (m_currentCallFrame == m_pauseOnCallFrame)
            m_pauseOnCallFrame = m_currentCallFrame->caller();
        m_currentCallFrame = m_currentCallFrame->caller();
        break;
    }
    
    case RUBY_EVENT_RAISE: {
        if (m_paused)
            return;

        // FIXME: How do we know if we have a Ruby exception handler?
        bool hasHandler = false;
        if (m_pauseOnExceptionsState == PauseOnAllExceptions || (m_pauseOnExceptionsState == PauseOnUncaughtExceptions && !hasHandler))
            m_pauseOnNextStatement = true;

        updateCurrentCallFrame();
        pauseIfNeeded();
        break;
    }

    case RUBY_EVENT_CLASS:
    case RUBY_EVENT_END:
    case RUBY_EVENT_C_CALL:
    case RUBY_EVENT_C_RETURN:
    default:
        break;
    }
}

bool RBScriptDebugServer::evaluateBreakpointCondition(const String& condition) const
{
    VALUE result = currentCallFrame()->evaluate(condition);
    return RTEST(result);
}

void RBScriptDebugServer::dispatchDidPause(ScriptDebugListener* listener)
{
    ASSERT(m_paused);
    m_doProcessEvents = false;
    VALUE rbCallFrame = toRB(m_currentCallFrame.get());
    VALUE binding = currentCallFrame()->binding();
    RBScriptState* state = RBScriptState::forBinding(binding);
    listener->didPause(state, RBScriptValue::scriptValue(rbCallFrame), ScriptValue());
    m_doProcessEvents = true;
}

void RBScriptDebugServer::dispatchDidParseSource(ListenerSet& listeners, const char* fileNamePtr)
{
    String fileName = fileNamePtr;
    if (!isValidUrl(fileName))
        return;
    
    String sourceID = String::number(RBDOMBinding::sourceIDFromFileName(fileNamePtr));

    ScriptDebugListener::Script script;
    script.url = fileName;
    script.endLine = INT_MAX;
    script.source = sourceForScriptUrl(script.url);
    script.isContentScript = false;

    setScriptSourceLinesAndColumns(script);

    m_callingListeners = true;
    
    Vector<ScriptDebugListener*> copy;
    copyToVector(listeners, copy);
    for (size_t i = 0; i < copy.size(); ++i)
        copy[i]->didParseSource(sourceID, script);

    m_callingListeners = false;
}

void RBScriptDebugServer::recompileAllJSFunctions(Timer<ScriptDebugServer>*)
{
    m_parsedScriptIds.clear();
}

void RBScriptDebugServer::didClearMainFrameWindowObject()
{
    m_parsedScriptIds.clear();
}

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)
