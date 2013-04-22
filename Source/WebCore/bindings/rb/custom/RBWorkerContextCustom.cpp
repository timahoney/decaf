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
#include "RBWorkerContext.h"

#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include "RBScheduledAction.h"

namespace WebCore {

VALUE RBWorkerContext::set_timeout(int argc, VALUE* argv, VALUE self)
{
    WorkerContext* selfImpl = impl<WorkerContext>(self);

    VALUE handler, timeoutRB;
    rb_scan_args(argc, argv, "02", &handler, &timeoutRB);
    if (rb_block_given_p()) {
        timeoutRB = handler;
        handler = rb_block_proc();
    }

    OwnPtr<ScheduledAction> action = RBScheduledAction::create(handler);
    if (!action)
        return INT2NUM(0);
    int delay = NIL_P(timeoutRB) ? 0 : NUM2INT(timeoutRB);
    int result = selfImpl->setTimeout(action.release(), delay);
    return toRB(result);
}

VALUE RBWorkerContext::set_interval(int argc, VALUE* argv, VALUE self)
{
    WorkerContext* selfImpl = impl<WorkerContext>(self);

    VALUE handler, timeoutRB;
    rb_scan_args(argc, argv, "02", &handler, &timeoutRB);
    if (rb_block_given_p()) {
        timeoutRB = handler;
        handler = rb_block_proc();
    }

    OwnPtr<ScheduledAction> action = RBScheduledAction::create(handler);
    if (!action)
        return INT2NUM(0);
    int delay = NIL_P(timeoutRB) ? 0 : NUM2INT(timeoutRB);
    int result = selfImpl->setInterval(action.release(), delay);
    return toRB(result);
}

VALUE RBWorkerContext::import_scripts(int argc, VALUE* argv, VALUE self)
{
    if (!argc)
        return Qnil;

    VALUE urlsRB;
    rb_scan_args(argc, argv, "*", &urlsRB);

    Vector<String> urls;
    unsigned length = RARRAY_LEN(urlsRB);
    for (unsigned i = 0; i < length; i++) {
        VALUE urlRB = rb_ary_entry(urlsRB, i);
        urls.append(rbToString(urlRB));
    }
    
    ExceptionCode ec = 0;
    WorkerContext* selfImpl = impl<WorkerContext>(self);
    selfImpl->importScripts(urls, ec);
    RB::setDOMException(ec);
    return Qnil;
}

} // namespace WebCore
