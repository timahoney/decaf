/*
 * Copyright (c) 2011 Google Inc. All rights reserved.
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

#include "config.h"

#if ENABLE(JAVASCRIPT_DEBUGGER)

#include "JSPageScriptDebugServer.h"

#include "EventLoop.h"
#include "Frame.h"
#include "FrameView.h"
#include "JSDOMWindowCustom.h"
#include "JSScriptCallFrame.h"
#include "Page.h"
#include "PageGroup.h"
#include "PluginView.h"
#include "ScriptDebugListener.h"
#include "Widget.h"
#include <runtime/JSLock.h>
#include <wtf/MainThread.h>
#include <wtf/OwnPtr.h>
#include <wtf/PassOwnPtr.h>
#include <wtf/StdLibExtras.h>

using namespace JSC;

namespace WebCore {

static Page* toPage(JSGlobalObject* globalObject)
{
    ASSERT_ARG(globalObject, globalObject);

    JSDOMWindow* window = asJSDOMWindow(globalObject);
    Frame* frame = window->impl()->frame();
    return frame ? frame->page() : 0;
}

JSPageScriptDebugServer& JSPageScriptDebugServer::shared()
{
    DEFINE_STATIC_LOCAL(JSPageScriptDebugServer, server, ());
    return server;
}

JSPageScriptDebugServer::JSPageScriptDebugServer()
    : ScriptDebugServer(JSScriptType)
    , PageScriptDebugServer(JSScriptType)
    , JSScriptDebugServer()
{
}

JSPageScriptDebugServer::~JSPageScriptDebugServer()
{
}

ScriptDebugServer::ListenerSet* JSPageScriptDebugServer::getListenersForGlobalObject(JSGlobalObject* globalObject)
{
    Page* page = toPage(globalObject);
    if (!page)
        return 0;
    return m_pageListenersMap.get(page);
}

void JSPageScriptDebugServer::recompileAllJSFunctions(Timer<ScriptDebugServer>*)
{
    JSLockHolder lock(JSDOMWindow::commonJSGlobalData());
    // If JavaScript stack is not empty postpone recompilation.
    if (JSDOMWindow::commonJSGlobalData()->dynamicGlobalObject)
        recompileAllJSFunctionsSoon();
    else
        Debugger::recompileAllJSFunctions(JSDOMWindow::commonJSGlobalData());
}

Page* JSPageScriptDebugServer::currentPage()
{
    JSC::JSGlobalObject* globalObject = static_cast<JSScriptCallFrame*>(m_currentCallFrame.get())->dynamicGlobalObject();
    return toPage(globalObject);
}

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)
