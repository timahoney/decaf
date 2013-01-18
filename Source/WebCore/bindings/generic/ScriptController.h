/*
 * Copyright (C) 2010 Research in Motion Limited. All rights reserved.
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 1999 Harri Porten (porten@kde.org)
 * Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
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

#ifndef ScriptController_h
#define ScriptController_h

#include "FrameLoaderTypes.h"
#include "KURL.h"
#include "ScriptDebugServer.h"
#include <wtf/Forward.h>
#include <wtf/Vector.h>

namespace WebCore {

enum ReasonForCallingCanExecuteScripts {
    AboutToExecuteScript,
    NotAboutToExecuteScript
};

class DOMWindow;
class DOMWrapperWorld;
class Frame;
class HTMLDocument;
class ScriptSourceCode;
class ScriptState;
class ScriptValue;
class SecurityOrigin;

class ScriptController {
public:
    virtual ~ScriptController();
    
    virtual void updateSecurityOrigin();
    virtual void updateDocument() = 0;

    virtual void enableEval() = 0;
    virtual void disableEval(const String& errorMessage) = 0;

    virtual void attachDebugger(ScriptDebugServer*) = 0;

    virtual void destroyWindowShell(DOMWrapperWorld*) = 0;
    virtual void clearScriptObjects() = 0;
    virtual void clearWindowShell(DOMWindow* newDOMWindow, bool goingIntoPageCache) = 0;

    virtual void namedItemAdded(HTMLDocument*, const AtomicString&);
    virtual void namedItemRemoved(HTMLDocument*, const AtomicString&);

    ScriptValue executeScript(const ScriptSourceCode&);
    ScriptValue executeScript(const String& script, bool forceUserGesture = false);
    ScriptValue executeScriptInWorld(DOMWrapperWorld*, const String& script, bool forceUserGesture = false);
    virtual ScriptValue evaluate(const ScriptSourceCode&) = 0;
    virtual ScriptValue evaluateInWorld(const ScriptSourceCode&, DOMWrapperWorld*) = 0;
    
    // Returns true if argument is a JavaScript URL.
    virtual bool executeIfJavaScriptURL(const KURL&, ShouldReplaceDocumentIfJavaScriptURL shouldReplaceDocumentIfJavaScriptURL = ReplaceDocumentIfJavaScriptURL);

#if ENABLE(INSPECTOR)
    virtual void collectIsolatedContexts(Vector<std::pair<ScriptState*, SecurityOrigin*> >&) = 0;
#endif

    // FIXME: Is this supposed to do anything?
    // It's being called, but is empty everywhere.
    virtual void updatePlatformScriptObjects();

    // This function must be called from the main thread. It is safe to call it repeatedly.
    // Darwin is an exception to this rule: it is OK to call this function from any thread, even reentrantly.
    static void initializeThreading();

    static bool processingUserGesture();
    static bool canAccessFromCurrentOrigin(Frame*);    
    static void getAllWorlds(Vector<RefPtr<DOMWrapperWorld> >&);
    static void setCaptureCallStackForUncaughtExceptions(bool set);

    bool canExecuteScripts(ReasonForCallingCanExecuteScripts);

    void setPaused(bool b) { m_paused = b; }
    bool isPaused() const { return m_paused; }
    const String* sourceURL() const { return m_sourceURL; } // 0 if we are not evaluating any script
    Frame* frame() const { return m_frame; };

    // FIXME: Stub for parity with V8 implementation. http://webkit.org/b/100815
    virtual bool shouldBypassMainWorldContentSecurityPolicy() { return false; }

protected:
    ScriptController(Frame*);

    Frame* m_frame;
    bool m_paused;
    const String* m_sourceURL;
};

} // namespace WebCore

#endif
