/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#ifndef JSScriptDebugServer_h
#define JSScriptDebugServer_h

#if ENABLE(JAVASCRIPT_DEBUGGER)

#include "ScriptDebugListener.h"
#include "ScriptDebugServer.h"
#include "ScriptBreakpoint.h"
#include "Timer.h"
#include <debugger/Debugger.h>
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>
#include <wtf/text/TextPosition.h>
#include <wtf/text/WTFString.h>

namespace JSC {
class DebuggerCallFrame;
class JSGlobalObject;
class ExecState;
}
namespace WebCore {

class JavaScriptCallFrame;
class ScriptDebugListener;
class ScriptObject;
class ScriptValue;

class JSScriptDebugServer : protected JSC::Debugger, public virtual ScriptDebugServer {
    WTF_MAKE_NONCOPYABLE(JSScriptDebugServer); WTF_MAKE_FAST_ALLOCATED;
public:
    virtual void attach(JSC::JSGlobalObject*);

protected:
    
    JSScriptDebugServer();
    virtual ~JSScriptDebugServer();

    virtual ListenerSet* getListenersForGlobalObject(JSC::JSGlobalObject*) = 0;

    virtual bool isContentScript(JSC::ExecState*);

    virtual bool evaluateBreakpointCondition(const String& condition) const;

    virtual void dispatchDidPause(ScriptDebugListener* listener);
    void dispatchDidParseSource(const ListenerSet& listeners, JSC::SourceProvider*, bool isContentScript);
    void dispatchFailedToParseSource(const ListenerSet& listeners, JSC::SourceProvider*, int errorLine, const String& errorMessage);

    void createCallFrame(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineNumber, int columnNumber);
    void updateCallFrameAndPauseIfNeeded(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineNumber, int columnNumber);

    virtual void detach(JSC::JSGlobalObject*);

    virtual void sourceParsed(JSC::ExecState*, JSC::SourceProvider*, int errorLine, const String& errorMsg);
    virtual void callEvent(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineNumber, int columnNumber);
    virtual void atStatement(const JSC::DebuggerCallFrame&, intptr_t sourceID, int firstLine, int columnNumber);
    virtual void returnEvent(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineNumber, int columnNumber);
    virtual void exception(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineNumber, int columnNumber, bool hasHandler);
    virtual void willExecuteProgram(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineno, int columnNumber);
    virtual void didExecuteProgram(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineno, int columnNumber);
    virtual void didReachBreakpoint(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineno, int columnNumber);
};

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)

#endif // JSScriptDebugServer_h
