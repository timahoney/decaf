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

#include "RBPageScriptDebugServer.h"

#include "DOMWindow.h"
#include "CachedResourceLoader.h"
#include "CachedScript.h"
#include "Frame.h"
#include "Page.h"
#include "RBDOMWindow.h"
#include "RBScriptCallFrame.h"
#include "RBScriptState.h"

namespace WebCore {
    
extern "C" void debug_event_hook(rb_event_flag_t event, VALUE data, VALUE self, ID mid, VALUE klass);
void debug_event_hook(rb_event_flag_t event, VALUE data, VALUE self, ID mid, VALUE klass)
{
    RBPageScriptDebugServer::shared().processEventHook(event, data, self, mid, klass);
}

RBPageScriptDebugServer& RBPageScriptDebugServer::shared()
{
    DEFINE_STATIC_LOCAL(RBPageScriptDebugServer, server, ());
    return server;
}

RBPageScriptDebugServer::RBPageScriptDebugServer()
    : ScriptDebugServer(RBScriptType)
    , PageScriptDebugServer(RBScriptType)
    , RBScriptDebugServer()
{
    rb_add_event_hook(debug_event_hook, (RUBY_EVENT_LINE | RUBY_EVENT_CALL | RUBY_EVENT_RETURN | RUBY_EVENT_RAISE), Qnil);
}
    
RBPageScriptDebugServer::~RBPageScriptDebugServer()
{
    rb_remove_event_hook(debug_event_hook);
}
    
void RBPageScriptDebugServer::processEventHook(rb_event_flag_t event, VALUE data, VALUE self, ID mid, VALUE klass)
{
    RBScriptDebugServer::processEventHook(event, data, self, mid, klass);
}

Page* RBPageScriptDebugServer::currentPage()
{
    VALUE binding = static_cast<RBScriptCallFrame*>(m_currentCallFrame.get())->binding();
    m_doProcessEvents = false;
    VALUE windowRB = rb_funcall(binding, rb_intern("eval"), 1, rb_str_new2("$window"));
    DOMWindow* window = impl<DOMWindow>(windowRB);
    m_doProcessEvents = true;
    if (window && window->frame())
        return window->frame()->page();
    
    return 0;
}

String RBPageScriptDebugServer::sourceForScriptUrl(String& url)
{
    if (url.isEmpty())
        return "";

    // FIXME: Is there a more reliable way to get the script source?
    Page* page = currentPage();
    CachedResource* resource = page->mainFrame()->document()->cachedResourceLoader()->cachedResource(url);
    CachedScript* script = static_cast<CachedScript*>(resource);
    if (!script)
        return "";
    return script->script();
}

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)
