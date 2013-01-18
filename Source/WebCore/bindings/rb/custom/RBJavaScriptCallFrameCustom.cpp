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

#include "RBJavaScriptCallFrame.h"

#include "RBConverters.h"
#include "RBDOMWindow.h"
#include "RBExceptionHandler.h"
#include "RBScriptCallFrame.h"

namespace WebCore {

#define RBIMPL impl<RBScriptCallFrame>(self)

VALUE RBJavaScriptCallFrame::evaluate(VALUE self, VALUE script)
{
    VALUE result = RBIMPL->evaluate(rbToString(script));
    return result;
}

VALUE RBJavaScriptCallFrame::restart(VALUE)
{
    // FIXME: Implement this.
    return Qfalse;
}

VALUE RBJavaScriptCallFrame::scope_chain_getter(VALUE self)
{
    VALUE scopeArray = rb_ary_new();
    RBScriptCallFrame* frame = RBIMPL;
    while (frame) {
        rb_ary_push(scopeArray, frame->binding());
        frame = static_cast<RBScriptCallFrame*>(frame->caller());
    }
    return scopeArray;
}

VALUE RBJavaScriptCallFrame::scope_type(VALUE self, VALUE index)
{
    int targetIndex = NUM2INT(index);
    RBScriptCallFrame* frame = RBIMPL;
    for (int i = 0; i <= targetIndex && frame; i++) {
        if (i < targetIndex)
            frame = static_cast<RBScriptCallFrame*>(frame->caller());
    }

    if (!frame)
        return INT2FIX(0);

    if (!frame->functionName().isEmpty())
        return INT2FIX(1); // local

    // FIXME: Return the correct scope type.
    return INT2FIX(0); // global
}

VALUE RBJavaScriptCallFrame::this_object_getter(VALUE self)
{
    return RBIMPL->thisObject();
}

VALUE RBJavaScriptCallFrame::type_getter(VALUE self)
{
    return rb_str_new2(RBIMPL->type().utf8().data());
}

VALUE RBJavaScriptCallFrame::set_variable_value(VALUE, VALUE, VALUE, VALUE)
{
    // FIXME: implement this. https://bugs.webkit.org/show_bug.cgi?id=107830
    rb_raise(rb_eTypeError, "Variable value mutation is not supported");
    return Qnil;
}

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)
