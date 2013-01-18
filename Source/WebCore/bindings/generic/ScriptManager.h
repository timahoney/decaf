/*
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

#ifndef ScriptManager_h
#define ScriptManager_h

#include "JSScriptController.h"
#include "RBScriptController.h"
#include <utility>
#include <wtf/Forward.h>
#include <wtf/HashMap.h>
#include <wtf/HashTraits.h>
#include <wtf/text/StringHash.h>

#define CALL_ALL_CONTROLLERS(defSig, callSig) \
void defSig \
{ \
    for (ScriptControllerMap::const_iterator it = m_controllers.begin(); it != m_controllers.end(); ++it) \
        it->value->callSig; \
}

#define CALL_ON_ALL(name) CALL_ALL_CONTROLLERS(name(), name());
#define CALL_ON_ALL1(name, type1, arg1) CALL_ALL_CONTROLLERS(name(type1 arg1), name(arg1));
#define CALL_ON_ALL2(name, type1, arg1, type2, arg2) CALL_ALL_CONTROLLERS(name(type1 arg1, type2 arg2), name(arg1, arg2));
#define CALL_ON_ALL3(name, type1, arg1, type2, arg2, type3, arg3) CALL_ALL_CONTROLLERS(name(type1 arg1, type2 arg2, type3 arg3), name(arg1, arg2, arg3));

namespace WebCore {

class Frame;
class DOMWindow;
class JSDOMWindowShell;
class ScriptController;
    
typedef HashMap<int, ScriptController*> ScriptControllerMap;

class ScriptManager {
public:
    ScriptManager(Frame*);
    virtual ~ScriptManager();
    
    ScriptController* scriptController(ScriptType);

    template<typename Function, class ...Args>
    void callOnAll(Function f, Args...args) 
    {
        for (ScriptControllerMap::const_iterator it = m_controllers.begin(); it != m_controllers.end(); ++it)
            (it->value->*f)(std::forward(args)...);
    }

    CALL_ON_ALL(updatePlatformScriptObjects)
    CALL_ON_ALL(updateSecurityOrigin)
    CALL_ON_ALL(updateDocument)
    CALL_ON_ALL(clearScriptObjects)
    CALL_ON_ALL(enableEval)
    CALL_ON_ALL1(disableEval, const String&, errorMessage)
    CALL_ON_ALL2(clearWindowShell, DOMWindow*, newDOMWindow, bool, goingIntoPageCache)
    CALL_ON_ALL2(namedItemAdded, HTMLDocument*, document, const AtomicString&, name)
    CALL_ON_ALL2(namedItemRemoved, HTMLDocument*, document, const AtomicString&, name)

    // FIXME: Make the DOMWindowShell generic.
    JSDOMWindowShell* existingWindowShell(DOMWrapperWorld*) const;

    // FIXME: Stub for parity with V8 implementation. http://webkit.org/b/100815
    bool shouldBypassMainWorldContentSecurityPolicy();

    // FIXME: Should canExecuteScripts be in both ScriptManager and ScriptController?
    bool canExecuteScripts(ReasonForCallingCanExecuteScripts);
    
protected:
    Frame* frame() const { return m_frame; };
    
private:
    Frame* m_frame;
    ScriptControllerMap m_controllers;

    JSScriptController m_jsController;
    RBScriptController m_rbController;
};

} // namespace WebCore

#endif
