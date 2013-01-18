/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "ScriptController.h"

#include "ContentSecurityPolicy.h"
#include "Document.h"
#include "DocumentLoader.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameLoaderClient.h"
#include "JSScriptController.h"
#include "JSScriptState.h"
#include "Page.h"
#include "RBScriptController.h"
#include "ScriptSourceCode.h"
#include "ScriptValue.h"
#include "SecurityOrigin.h"
#include "Settings.h"
#include "UserGestureIndicator.h"
#include <runtime/InitializeThreading.h>
#include <wtf/text/TextPosition.h>

namespace WebCore {
    
#define CALL_ALL_STATIC(defSig, callSig) \
void ScriptController::defSig \
{ \
JSScriptController::callSig; \
RBScriptController::callSig; \
}
    
#define CALL_STATIC(name) CALL_ALL_STATIC(name(), name());
#define CALL_STATIC1(name, type1, arg1) CALL_ALL_STATIC(name(type1 arg1), name(arg1));
    
ScriptController::ScriptController(Frame* frame)
: m_frame(frame)
, m_paused(false)
, m_sourceURL(0)
{
}

ScriptController::~ScriptController()
{
}

void ScriptController::initializeThreading()
{
    JSC::initializeThreading();
    WTF::initializeMainThread();
}

bool ScriptController::processingUserGesture()
{
    return UserGestureIndicator::processingUserGesture();
}

bool ScriptController::canExecuteScripts(ReasonForCallingCanExecuteScripts reason)
{
    if (m_frame->document() && m_frame->document()->isSandboxed(SandboxScripts)) {
        // FIXME: This message should be moved off the console once a solution to https://bugs.webkit.org/show_bug.cgi?id=103274 exists.
        if (reason == AboutToExecuteScript)
            m_frame->document()->addConsoleMessage(SecurityMessageSource, ErrorMessageLevel, "Blocked script execution in '" + m_frame->document()->url().elidedString() + "' because the document's frame is sandboxed and the 'allow-scripts' permission is not set.");
        return false;
    }

    if (m_frame->document() && m_frame->document()->isViewSource()) {
        ASSERT(m_frame->document()->securityOrigin()->isUnique());
        return true;
    }
    
    Settings* settings = m_frame->settings();
    const bool allowed = m_frame->loader()->client()->allowScript(settings && settings->isScriptEnabled());
    if (!allowed && reason == AboutToExecuteScript)
        m_frame->loader()->client()->didNotAllowScript();
    return allowed;
}

ScriptValue ScriptController::executeScript(const String& script, bool forceUserGesture)
{
    UserGestureIndicator gestureIndicator(forceUserGesture ? DefinitelyProcessingNewUserGesture : PossiblyProcessingUserGesture);
    return executeScript(ScriptSourceCode(script, m_frame->document()->url()));
}

ScriptValue ScriptController::executeScript(const ScriptSourceCode& sourceCode)
{
    if (!canExecuteScripts(AboutToExecuteScript) || isPaused())
        return ScriptValue();

    RefPtr<Frame> protect(m_frame); // Script execution can destroy the frame, and thus the ScriptController.

    return evaluate(sourceCode);
}

ScriptValue ScriptController::executeScriptInWorld(DOMWrapperWorld* world, const String& script, bool forceUserGesture)
{
    UserGestureIndicator gestureIndicator(forceUserGesture ? DefinitelyProcessingNewUserGesture : PossiblyProcessingUserGesture);
    ScriptSourceCode sourceCode(script, m_frame->document()->url());

    if (!canExecuteScripts(AboutToExecuteScript) || isPaused())
        return ScriptValue();

    return evaluateInWorld(sourceCode, world);
}
    
bool ScriptController::executeIfJavaScriptURL(const KURL&, ShouldReplaceDocumentIfJavaScriptURL)
{
    // FIXME: Even though the JSScriptController overrides this function, this is still weird.
    // The only reason this will be called is if a non-JS ScriptController is used.
    // Maybe this can be changed if we ever allow "ruby:" URLs.
    return false;
}
    
CALL_STATIC1(getAllWorlds, Vector<RefPtr<DOMWrapperWorld> >&, worlds)
CALL_STATIC1(setCaptureCallStackForUncaughtExceptions, bool, set)
    
void ScriptController::updatePlatformScriptObjects() { }
void ScriptController::updateSecurityOrigin() { }
void ScriptController::namedItemAdded(HTMLDocument*, const AtomicString&) { }
void ScriptController::namedItemRemoved(HTMLDocument*, const AtomicString&) { }

} // namespace WebCore
