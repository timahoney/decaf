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

#ifndef RBScriptDebugServer_h
#define RBScriptDebugServer_h

#if ENABLE(JAVASCRIPT_DEBUGGER)

#include "ScriptDebugServer.h"
#include <Ruby/ruby.h>
#include <wtf/HashSet.h>

namespace WebCore {

class JavaScriptCallFrame;
class RBScriptCallFrame;
class ScriptDebugListener;
class ScriptObject;
class ScriptValue;

class RBScriptDebugServer : public virtual ScriptDebugServer {
    WTF_MAKE_NONCOPYABLE(RBScriptDebugServer); WTF_MAKE_FAST_ALLOCATED;
public:
    virtual void recompileAllJSFunctions(Timer<ScriptDebugServer>*);

    void processEventHook(rb_event_flag_t event, VALUE data, VALUE self, ID mid, VALUE klass);
    
    virtual void didClearMainFrameWindowObject();

protected:
    RBScriptDebugServer();
    virtual ~RBScriptDebugServer();

    virtual bool evaluateBreakpointCondition(const String& condition) const;

    virtual void dispatchDidPause(ScriptDebugListener*);
    void dispatchDidParseSource(ListenerSet& listeners, const char* fileNamePtr);

    virtual String sourceForScriptUrl(String& url) = 0;
    
    bool m_doProcessEvents;
    HashSet<intptr_t> m_parsedScriptIds;

private:
    void createCurrentCallFrame();
    void updateCurrentCallFrame();
    RBScriptCallFrame* currentCallFrame() const;
};

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)

#endif // RBScriptDebugServer_h
