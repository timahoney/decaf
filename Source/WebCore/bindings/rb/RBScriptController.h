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

#ifndef RBScriptController_h
#define RBScriptController_h

#include "ScriptController.h"
#include <Ruby/ruby.h>

namespace WebCore {

class RBScriptController : public ScriptController {
public:
    RBScriptController(Frame* frame);

    virtual ~RBScriptController();

    virtual void updateDocument();

    virtual void enableEval();
    virtual void disableEval(const String& errorMessage);
    virtual void attachDebugger(ScriptDebugServer*);
    virtual void destroyWindowShell(DOMWrapperWorld*);
    virtual void clearScriptObjects();
    virtual void clearWindowShell(DOMWindow* newDOMWindow, bool goingIntoPageCache);

    virtual ScriptValue evaluate(const ScriptSourceCode&);
    virtual ScriptValue evaluateInWorld(const ScriptSourceCode&, DOMWrapperWorld*);

    static void getAllWorlds(Vector<RefPtr<DOMWrapperWorld> >&);

#if ENABLE(INSPECTOR)
    static void setCaptureCallStackForUncaughtExceptions(bool);
    virtual void collectIsolatedContexts(Vector<std::pair<ScriptState*, SecurityOrigin*> >&);
#endif
};

} // namespace WebCore

#endif
