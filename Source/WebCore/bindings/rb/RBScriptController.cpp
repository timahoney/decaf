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

#include "config.h"
#include "RBScriptController.h"

#include "DOMWindow.h"
#include "Document.h"
#include "Frame.h"
#include "RBCallHelpers.h"
#include "RBConverters.h"
#include "RBDOMBinding.h"
#include "RBDOMWindow.h"
#include "RBDocument.h"
#include "RBInitialization.h"
#include "RBInitializationCustom.h"
#include "RBSecurityMeasures.h"
#include "RBScriptState.h"
#include "RBScriptValue.h"
#include "ScriptSourceCode.h"
#include <Ruby/ruby.h>
#include <wtf/Assertions.h>
#include <wtf/Forward.h>
#include <wtf/HashSet.h>
#include <wtf/text/CString.h>
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>

using namespace RB;
    
namespace WebCore {

using namespace RB;

static VALUE globalWindowGetter(ID, VALUE*, struct global_entry*)
{
    return RBDOMBinding::currentWindowRB();
}
    
static void initRuby()
{
    static bool initializedRuby = false;
    if (initializedRuby)
        return;
    initializedRuby = true;
    
    RUBY_INIT_STACK
    ruby_init();

    ensureRubySecurity();
    RBInitialization::initializeRubyClasses();
    RBInitializationCustom::initializeCustomRubyClasses();

    rb_define_virtual_variable("window", RUBY_METHOD_FUNC(&globalWindowGetter), 0);
}

RBScriptController::RBScriptController(Frame* frame)
    : ScriptController(frame)
{
    initRuby();
}

RBScriptController::~RBScriptController()
{
}

ScriptValue RBScriptController::evaluate(const ScriptSourceCode& source)
{
    VALUE binding = bindingFromContext(frame()->document());
    VALUE scriptString = rb_str_new2(source.source().utf8().data());
    VALUE fileName = rb_str_new2(source.url().string().utf8().data());
    VALUE lineNumber = INT2NUM(1);
    VALUE args[3];
    args[0] = scriptString;
    args[1] = fileName;
    args[2] = lineNumber;
    VALUE result = callFunctionProtected(binding, "eval", 3, args);

    VALUE exception = rb_errinfo();
    if (!NIL_P(exception)) {
        rb_set_errinfo(Qnil);
        RBDOMBinding::reportException(frame()->document(), exception);
        return ScriptValue();
    }

    return RBScriptValue::scriptValue(result);
}
    
ScriptValue RBScriptController::evaluateInWorld(const ScriptSourceCode& source, DOMWrapperWorld*)
{
    // FIXME: Implement this for real when we have an RBDOMWrapperWorld.
    return evaluate(source);
}

void RBScriptController::attachDebugger(ScriptDebugServer*)
{
    // FIXME: Implement this.
}

// FIXME: Implement this once we have the Ruby DOMWrapperWorld.
void RBScriptController::getAllWorlds(Vector<RefPtr<DOMWrapperWorld> >&) { }

void RBScriptController::updateDocument()
{
    // FIXME: What do we do here? Should we clear stuff from the previous window?
}

// FIXME: Do these even matter anywhere? Can we delete it from ScriptController?
void RBScriptController::enableEval() { }
void RBScriptController::disableEval(const String&) { }

// FIXME: What do we do here?
void RBScriptController::destroyWindowShell(DOMWrapperWorld*) { }
void RBScriptController::clearScriptObjects() { }
void RBScriptController::clearWindowShell(DOMWindow*, bool) { }

#if ENABLE(INSPECTOR)
// FIXME: What to do here?
void RBScriptController::setCaptureCallStackForUncaughtExceptions(bool) { }

// FIXME: Do something once we get window shells for Ruby.
void RBScriptController::collectIsolatedContexts(Vector<std::pair<ScriptState*, SecurityOrigin*> >&) { }
#endif

} // namespace WebCore
