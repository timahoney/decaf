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

#if ENABLE(DEVICE_ORIENTATION)

#include "RBDeviceOrientationEvent.h"

namespace WebCore {

VALUE RBDeviceOrientationEvent::alpha(VALUE self)
{
    DeviceOrientationEvent* impl = impl<DeviceOrientationEvent>(self);
    if (!impl->orientation()->canProvideAlpha())
        return Qnil;
    return toRB(impl->orientation()->alpha());
}

VALUE RBDeviceOrientationEvent::beta(VALUE self)
{
    DeviceOrientationEvent* impl = impl<DeviceOrientationEvent>(self);
    if (!impl->orientation()->canProvideBeta())
        return Qnil;
    return toRB(impl->orientation()->beta());
}

VALUE RBDeviceOrientationEvent::gamma(VALUE self)
{
    DeviceOrientationEvent* impl = impl<DeviceOrientationEvent>(self);
    if (!impl->orientation()->canProvideGamma())
        return Qnil;
    return toRB(impl->orientation()->gamma());
}

VALUE RBDeviceOrientationEvent::absolute(VALUE self)
{
    DeviceOrientationEvent* impl = impl<DeviceOrientationEvent>(self);
    if (!impl->orientation()->canProvideAbsolute())
        return Qnil;
    return toRB(impl->orientation()->absolute());
}

VALUE RBDeviceOrientationEvent::init_device_orientation_event(VALUE self, VALUE rbType, VALUE rbBubbles, VALUE rbCancelable, VALUE rbAlpha, VALUE rbBeta, VALUE rbGamma, VALUE rbAbsolute)
{
    // FIXME: This should be a static/class method in Ruby, should it not?

    const String type = StringValueCStr(rbType);
    bool bubbles = RTEST(rbBubbles);
    bool cancelable = RTEST(rbCancelable);

    bool alphaProvided = NIL_P(rbAlpha);
    double alpha = alphaProvided ? NUM2DBL(rbAlpha) : 0;
    bool betaProvided = NIL_P(rbBeta);
    double beta = betaProvided ? NUM2DBL(rbBeta) : 0;
    bool gammaProvided = NIL_P(rbGamma);
    double gamma = gammaProvided ? NUM2DBL(rbGamma) : 0;
    bool absoluteProvided = NIL_P(rbAbsolute);
    bool absolute = absoluteProvided ? RTEST(absolute) : false;
    RefPtr<DeviceOrientationData> orientation = DeviceOrientationData::create(alphaProvided, alpha, betaProvided, beta, gammaProvided, gamma, absoluteProvided, absolute);
    DeviceOrientationEvent* impl = impl<DeviceOrientationEvent>(self);
    impl->initDeviceOrientationEvent(type, bubbles, cancelable, orientation.get());
    return Qnil;
}
    
} // namespace WebCore

#endif // ENABLE(DEVICE_ORIENTATION)
