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
#include "RBDOMBinding.h"

#include "Document.h"
#include "Frame.h"
#include "RBCallHelpers.h"
#include "RBConverters.h"
#include "RBDedicatedWorkerContext.h"
#include "RBDOMCoreException.h"
#include "RBDOMWindow.h"
#include "RBEventException.h"
#include "RBObject.h"
#include "RBRangeException.h"
#include "RBXMLHttpRequestException.h"
#if ENABLE(SHARED_WORKERS)
#include "RBSharedWorkerContext.h"
#endif
#if ENABLE(SQL_DATABASE)
#include "RBSQLException.h"
#endif
#if ENABLE(SVG)
#include "RBSVGException.h"
#endif
#include "RBXPathException.h"
#include "RBWorkerContext.h"
#include <Ruby/intern.h>

namespace RB {

using namespace WebCore;

typedef HashMap<ScriptExecutionContext*, VALUE> RBContextToBindingMap;
static RBContextToBindingMap* contextBindings;
typedef HashMap<VALUE, ScriptExecutionContext*> RBModuleToContextMap;
static RBModuleToContextMap* moduleContexts;
static VALUE savedBindings;

static void initializeMaps()
{
    contextBindings = new RBContextToBindingMap();
    moduleContexts = new RBModuleToContextMap();
    savedBindings = rb_ary_new();
    rb_gc_register_address(&savedBindings);
}

static ScriptExecutionContext* contextFromModule(VALUE module)
{
    if (!moduleContexts)
        initializeMaps();
    
    ScriptExecutionContext* context = moduleContexts->get(module);
    if (!context) {        
        VALUE attached = rb_iv_get(module, "__attached__");
        if (NIL_P(attached)) {
            // FIXME: Sometimes we lose the top instance_eval context from the console.
            // Find a way to make sure that it's always there.
            // This is probably unsafe.
            attached = rb_iv_get(module, "@inspected_window");
        }
        VALUE klass = rb_obj_class(attached);
        if (klass == RBDOMWindow::rubyClass())
            context = impl<DOMWindow>(attached)->document();
        else if (klass == RBWorkerContext::rubyClass())
            context = impl<WorkerContext>(attached);

        moduleContexts->set(module, context);
    }

    return context;
}

ScriptExecutionContext* contextFromBinding(VALUE binding)
{
    // We can always find our topmost module by calling the 'nesting'
    // function on the Module class. Since the top module is created
    // using an instance_eval, it will have an __attached__ instance variable,
    // which will be the context we are looking for.
    VALUE nesting = rb_funcall(binding, rb_intern("eval"), 1, rb_str_new2("Module.nesting"));
    ASSERT(!NIL_P(nesting) && RARRAY_LEN(nesting) > 0);
    VALUE topModule = rb_ary_entry(nesting, RARRAY_LEN(nesting) - 1);
    return contextFromModule(topModule);
}

ScriptExecutionContext* currentContext()
{
    // See contextFromBinding for a description of nesting.
    VALUE nesting = rb_funcall(rb_cModule, rb_intern("nesting"), 0);
    ASSERT(!NIL_P(nesting) && RARRAY_LEN(nesting) > 0);
    VALUE topModule = rb_ary_entry(nesting, RARRAY_LEN(nesting) - 1);
    return contextFromModule(topModule);
}

VALUE bindingFromContext(ScriptExecutionContext* context)
{
    if (!contextBindings)
        initializeMaps();

    VALUE binding = contextBindings->get(context);
    if (!RTEST(binding)) {

        // The code for each execution context (Window or Worker) 
        // is evaluating in it's own binding.
        // This binding exists in an instance_eval on the context.
        // Each instance_eval creates an anonymous module and runs
        // the code inside that module. This creates a nice little sandbox.
        // Any user-defined classes are confined in the sandbox and
        // cannot be accessed from other contexts.
        
        VALUE contextRB = Qnil;
        if (context->isDocument())
            contextRB = toRB(static_cast<Document*>(context)->domWindow());
#if ENABLE(WORKERS)
        else if (context->isWorkerContext())
            contextRB = toRB(static_cast<WorkerContext*>(context));
#endif

        ASSERT(!NIL_P(contextRB));

        binding = rb_funcall(contextRB, rb_intern("instance_eval"), 1, rb_str_new2("binding"));
        rb_ary_push(savedBindings, binding);
        contextBindings->set(context, binding);
    }

    return binding;
}

static ExceptionBase* toExceptionBase(VALUE value) 
{
    if (IS_RB_KIND(value, DOMCoreException))
        return impl<DOMCoreException>(value);
    if (IS_RB_KIND(value, RangeException))
        return impl<RangeException>(value);
    if (IS_RB_KIND(value, EventException))
        return impl<EventException>(value);
    if (IS_RB_KIND(value, XMLHttpRequestException))
        return impl<XMLHttpRequestException>(value);
#if ENABLE(SVG)
    if (IS_RB_KIND(value, SVGException))
        return impl<SVGException>(value);
#endif
    if (IS_RB_KIND(value, XPathException))
        return impl<XPathException>(value);
#if ENABLE(SQL_DATABASE)
    if (IS_RB_KIND(value, SQLException))
        return impl<SQLException>(value);
#endif

    return 0;
}

void reportException(ScriptExecutionContext* scriptExecutionContext, VALUE exception, CachedScript* cachedScript)
{
    if (NIL_P(exception))
        return;

    VALUE backtrace = rb_funcall(exception, rb_intern("backtrace"), 0);
    VALUE backtrace1 = rb_ary_entry(backtrace, 0);
    String backtraceString = rbToString(backtrace1);
    int indexSecondColon = backtraceString.reverseFind(":");
    int indexFirstColon = backtraceString.reverseFind(":", indexSecondColon - 1);
    String exceptionSourceURL = backtraceString.substring(0, indexFirstColon);
    int lineNumber = backtraceString.substring(indexFirstColon + 1, indexSecondColon - indexFirstColon - 1).toInt();
    
    String errorMessage = rbToString(rb_funcall(exception, rb_intern("inspect"), 0));
    if (ExceptionBase* exceptionBase = toExceptionBase(exception))
        errorMessage = exceptionBase->message() + ": " + exceptionBase->description();

    scriptExecutionContext->reportException(errorMessage, lineNumber, exceptionSourceURL, 0, cachedScript);
}

void reportCurrentException(RBScriptState* state, CachedScript* cachedScript)
{
    VALUE exception = rb_errinfo();
    rb_set_errinfo(Qnil);

    if (state->domWindow() && !state->domWindow()->isCurrentlyDisplayedInFrame())
        return;

    reportException(state->scriptExecutionContext(), exception, cachedScript);
}

typedef HashMap<String, intptr_t> RBFileNameToSourceIDMap;
static RBFileNameToSourceIDMap* fileNameSourceIDs;

intptr_t sourceIDFromFileName(const char* fileName)
{
    if (!fileNameSourceIDs)
        fileNameSourceIDs = new RBFileNameToSourceIDMap();

    String fileNameString = fileName;
    intptr_t id = fileNameSourceIDs->get(fileNameString);
    if (!id) {
        id = StringHasher::computeHash(fileNameString.characters());
        fileNameSourceIDs->set(fileNameString, id);
    }

    return id;
}

} // namespace RB
