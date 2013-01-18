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

#if ENABLE(GEOLOCATION)

#include "RBGeolocation.h"

#include "RBConverters.h"
#include "RBPositionCallback.h"
#include "RBPositionErrorCallback.h"

namespace WebCore {

static PassRefPtr<PositionOptions> createPositionOptions(VALUE options)
{
    RefPtr<PositionOptions> positionOptions = PositionOptions::create();
    if (!NIL_P(options)) {
        if (!IS_RB_HASH(options)) {
            rb_raise(rb_eTypeError, "The options argument to get_current_position must be a hash.");
            return 0;
        }

        VALUE enableHighAccuracy = rb_hash_aref(options, ID2SYM(rb_intern("enable_high_accuracy")));
        if (!NIL_P(enableHighAccuracy))
            positionOptions->setEnableHighAccuracy(RTEST(enableHighAccuracy));

        VALUE timeout = rb_hash_aref(options, ID2SYM(rb_intern("timeout")));
        if (!NIL_P(timeout))
            positionOptions->setTimeout(NUM2INT(timeout));

        VALUE maximumAge = rb_hash_aref(options, ID2SYM(rb_intern("maximum_age")));
        if (!NIL_P(maximumAge))
            positionOptions->setMaximumAge(NUM2INT(maximumAge));
    }

    return positionOptions.release();
}

VALUE RBGeolocation::get_current_position(int argc, VALUE* argv, VALUE self)
{
    Geolocation* selfImpl = impl<Geolocation>(self);
    RBScriptState* state = RBScriptState::current();
    VALUE successCallback, errorCallback, options;
    rb_scan_args(argc, argv, "03", &successCallback, &errorCallback, &options);

    if (rb_block_given_p()) {
        options = errorCallback;
        errorCallback = successCallback;
        successCallback = rb_block_proc();
    }

    if (NIL_P(successCallback)) {
        rb_raise(rb_eArgError, "get_current_position requires a Proc argument or an implicit block.");
        return Qnil;
    }

    RefPtr<PositionCallback> positionCallback = RBPositionCallback::create(successCallback, state->scriptExecutionContext());
    RefPtr<PositionErrorCallback> positionErrorCallback;
    if (!NIL_P(errorCallback))
        positionErrorCallback = RBPositionErrorCallback::create(errorCallback, state->scriptExecutionContext());
    RefPtr<PositionOptions> positionOptions = createPositionOptions(options);
    if (!positionOptions)
        return Qnil;

    selfImpl->getCurrentPosition(positionCallback.release(), positionErrorCallback.release(), positionOptions.release());
    return Qnil;
}

VALUE RBGeolocation::watch_position(int argc, VALUE* argv, VALUE self)
{
    Geolocation* selfImpl = impl<Geolocation>(self);
    RBScriptState* state = RBScriptState::current();
    VALUE successCallback, errorCallback, options;
    rb_scan_args(argc, argv, "03", &successCallback, &errorCallback, &options);

    if (rb_block_given_p()) {
        options = errorCallback;
        errorCallback = successCallback;
        successCallback = rb_block_proc();
    }

    if (NIL_P(successCallback)) {
        rb_raise(rb_eArgError, "get_current_position requires a Proc argument or an implicit block.");
        return Qnil;
    }

    RefPtr<PositionCallback> positionCallback = RBPositionCallback::create(successCallback, state->scriptExecutionContext());
    RefPtr<PositionErrorCallback> positionErrorCallback;
    if (!NIL_P(errorCallback))
        positionErrorCallback = RBPositionErrorCallback::create(errorCallback, state->scriptExecutionContext());
    RefPtr<PositionOptions> positionOptions = createPositionOptions(options);
    if (!positionOptions)
        return Qnil;

    selfImpl->watchPosition(positionCallback.release(), positionErrorCallback.release(), positionOptions.release());
    return Qnil;
}

#endif // ENABLE(GEOLOCATION)

} // namespace WebCore
