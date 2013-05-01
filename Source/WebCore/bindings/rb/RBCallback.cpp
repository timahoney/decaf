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
#include "RBCallback.h"

#include "RBCallHelpers.h"
#include "RBDOMBinding.h"
#include <wtf/text/CString.h>

using namespace RB;

namespace WebCore {

RBCallback::RBCallback(VALUE proc)
{
    VALUE klass = CLASS_OF(proc);
    if (klass == rb_cProc || klass == rb_cMethod) {
        m_proc = proc;
        rb_gc_register_address(&m_proc);
    } else {
        m_proc = Qnil;
    }
}

RBCallback::~RBCallback()
{
    rb_gc_unregister_address(&m_proc);
}

VALUE RBCallback::call(ScriptExecutionContext* scriptExecutionContext, int argc, VALUE* argv)
{
    if (NIL_P(m_proc))
        return Qnil;
    
    // FIXME: Right now, a Proc can take less than
    // the number of arguments called here, but a Method
    // will crash. For example, window.onload = Proc.new {} works,
    // but window.onload = method(:a) will crash if 'a' takes no arguments.
    // Do we want to allow Methods to specify less arguments?
    VALUE exception;
    VALUE result = callFunction(m_proc, "call", argc, argv, &exception);

    if (!NIL_P(exception))
        RBDOMBinding::reportException(scriptExecutionContext, exception);

    return result;
}

bool RBCallback::operator==(const RBCallback& other)
{
    return m_proc == other.m_proc;
}

} // namespace WebCore
