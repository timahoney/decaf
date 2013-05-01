/*
 * Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
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
#include "RBCallHelpers.h"

#include <Ruby/ruby.h>

namespace WebCore {

static VALUE printException(VALUE exceptionObject)
{
    printf("--EXCEPTION--\n");
    printf("Type:\t\t%s\n", rb_obj_classname(exceptionObject));

    VALUE message = rb_funcall(exceptionObject, rb_intern("message"), 0);
    printf("Message:\t%s\n", StringValueCStr(message));
    VALUE backtrace = rb_funcall(exceptionObject, rb_intern("backtrace"), 0);

    printf("Backtrace:\t");
    int count = NUM2INT(rb_funcall(backtrace, rb_intern("size"), 0));
    for (int i = 0; i < count; i++) {
        if (i > 0)
            printf("\t\t\t");
        VALUE backtraceLine = rb_funcall(backtrace, rb_intern("[]"), 1, INT2NUM(i));
        printf("%s\n", StringValueCStr(backtraceLine));
    }

    return Qnil;
}

extern "C" VALUE call_protected(VALUE);
VALUE call_protected(VALUE callParams)
{
    VALUE object = rb_ary_shift(callParams);
    VALUE rbFunctionName = rb_ary_shift(callParams);
    char* functionName = StringValueCStr(rbFunctionName);
    return rb_funcall2(object, rb_intern(functionName), RARRAY_LEN(callParams), RARRAY_PTR(callParams));
}

VALUE callFunctionProtected(VALUE obj, const char* functionName, long argc, const VALUE* argv)
{
    VALUE callParams = rb_ary_new();
    rb_ary_push(callParams, obj);
    rb_ary_push(callParams, rb_str_new2(functionName));
    for (int i = 0; i < argc; i++)
        rb_ary_push(callParams, argv[i]);
    
    int exception = 0;
    rb_gc_register_address(&callParams);
    VALUE result = rb_protect(&call_protected, callParams, &exception);
    rb_gc_unregister_address(&callParams);
    if (exception)
        printException(rb_errinfo());
    
    return result;
}

} // namespace WebCore

using namespace WebCore;

namespace RB {

class RBFunctionRunner {
public:
    RBFunctionRunner(VALUE object, const char* functionName, VALUE parameters)
        : m_object(object)
        , m_functionName(rb_intern(functionName))
        , m_parameters(parameters)
        , m_exception(Qnil)
    {
        rb_gc_register_address(&m_object);
        rb_gc_register_address(&m_parameters);
    }

    ~RBFunctionRunner()
    {
        rb_gc_unregister_address(&m_object);
        rb_gc_unregister_address(&m_parameters);
    }

    VALUE callFunction()
    {
        VALUE result = rb_rescue(RUBY_METHOD_FUNC(&call), (VALUE) this, RUBY_METHOD_FUNC(&rescue), (VALUE) this);
        if (!NIL_P(m_exception))
            printException(m_exception);
    
        return result;
    }

    VALUE exception() const { return m_exception; }

private:
    static VALUE rescue(RBFunctionRunner* runner, VALUE exception_object)
    {
        runner->m_exception = exception_object;
        return Qnil;
    }

    static VALUE call(RBFunctionRunner* runner)
    {
        return rb_funcall2(runner->m_object, runner->m_functionName, RARRAY_LEN(runner->m_parameters), RARRAY_PTR(runner->m_parameters));
    }

    VALUE m_object;
    ID m_functionName;
    VALUE m_parameters;
    VALUE m_exception;
};

static VALUE callFunctionInternal(VALUE object, const char* functionName, VALUE* exception, int argc, ...)
{
    VALUE *argv = 0;
    va_list argList;

    if (argc > 0) {
        va_start(argList, argc);
        argv = ALLOCA_N(VALUE, argc);
        for (int i = 0; i < argc; i++)
            argv[i] = va_arg(argList, VALUE);

        va_end(argList);
    }

    VALUE parameters = rb_ary_new4(argc, argv);
    RBFunctionRunner runner(object, functionName, parameters);
    VALUE result = runner.callFunction();
    if (exception)
        *exception = runner.exception();

    return result;
}

VALUE callFunction(VALUE obj, const char* functionName, VALUE* exception)
{
    return callFunctionInternal(obj, functionName, exception, 0);
}

VALUE callFunction(VALUE obj, const char* functionName, VALUE arg1, VALUE* exception)
{
    return callFunctionInternal(obj, functionName, exception, 1, arg1);
}

VALUE callFunction(VALUE obj, const char* functionName, VALUE arg1, VALUE arg2, VALUE* exception)
{
    return callFunctionInternal(obj, functionName, exception, 2, arg1, arg2);
}

VALUE callFunction(VALUE obj, const char* functionName, VALUE arg1, VALUE arg2, VALUE arg3, VALUE* exception)
{
    return callFunctionInternal(obj, functionName, exception, 3, arg1, arg2, arg3);
}

VALUE callFunction(VALUE obj, const char* functionName, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4, VALUE* exception)
{
    return callFunctionInternal(obj, functionName, exception, 4, arg1, arg2, arg3, arg4);
}

} // namespace RB
