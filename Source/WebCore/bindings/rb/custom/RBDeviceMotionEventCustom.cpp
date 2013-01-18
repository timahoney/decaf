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

#include "RBDeviceMotionEvent.h"

namespace WebCore {

static PassRefPtr<DeviceMotionData::Acceleration> readAccelerationArgument(VALUE object)
{
    if (NIL_P(object))
        return 0;

    VALUE xValue = rb_hash_aref(object, ID2SYM(rb_intern("x")));
    bool canProvideX = !NIL_P(xValue);
    double x = canProvideX ? NUM2DBL(xValue) : 0;

    VALUE yValue = rb_hash_aref(object, ID2SYM(rb_intern("y")));
    bool canProvideY = !NIL_P(yValue);
    double y = canProvideY ? NUM2DBL(yValue) : 0;

    VALUE zValue = rb_hash_aref(object, ID2SYM(rb_intern("z")));
    bool canProvideZ = !NIL_P(zValue);
    double y = canProvideZ ? NUM2DBL(zValue) : 0;

    if (!canProvideX && !canProvideY && !canProvideZ)
        return 0;

    return DeviceMotionData::Acceleration::create(canProvideX, x, canProvideY, y, canProvideZ, z);
}

static PassRefPtr<DeviceMotionData::RotationRate> readRotationRateArgument(VALUE object)
{
    if (NIL_P(object))
        return 0;

    VALUE alphaValue = rb_hash_aref(object, ID2SYM(rb_intern("alpha")));
    bool canProvideAlpha = !NIL_P(alphaValue);
    double alpha = canProvideAlpha ? NUM2DBL(alphaValue) : 0;

    VALUE betaValue = rb_hash_aref(object, ID2SYM(rb_intern("beta")));
    bool canProvideBeta = !NIL_P(betaValue);
    double beta = canProvideBeta ? NUM2DBL(betaValue) : 0;

    VALUE gammaValue = rb_hash_aref(object, ID2SYM(rb_intern("gamma")));
    bool canProvideGamma = !NIL_P(gammaValue);
    double gamma = canProvideGamma ? NUM2DBL(gammaValue) : 0;

    if (!canProvideAlpha && !canProvideBeta && !canProvideGamma)
        return 0;

    return DeviceMotionData::RotationRate::create(canProvideAlpha, alpha, canProvideBeta, beta, canProvideGamma, gamma);
}

static VALUE createAccelerationObject(const DeviceMotionData::Acceleration* acceleration)
{
    VALUE hash = rb_hash_new();
    rb_hash_aset(hash, ID2SYM(rb_intern("x")), acceleration->canProvideX() ? toRB(acceleration->x()) : Qnil);
    rb_hash_aset(hash, ID2SYM(rb_intern("y")), acceleration->canProvideY() ? toRB(acceleration->y()) : Qnil);
    rb_hash_aset(hash, ID2SYM(rb_intern("z")), acceleration->canProvideZ() ? toRB(acceleration->z()) : Qnil);
    return hash;
}

static VALUE createRotationRateObject(const DeviceMotionData::RotationRate* rotationRate)
{
    VALUE hash = rb_hash_new();
    rb_hash_aset(hash, ID2SYM(rb_intern("alpha")), rotationRate->canProvideAlpha() ? toRB(rotationRate->alpha()) : Qnil);
    rb_hash_aset(hash, ID2SYM(rb_intern("beta")), rotationRate->canProvideBeta() ? toRB(rotationRate->beta()) : Qnil);
    rb_hash_aset(hash, ID2SYM(rb_intern("gamma")), rotationRate->canProvideGamma() ? toRB(rotationRate->gamma()) : Qnil);
    return hash;
}

VALUE RBDeviceMotionEvent::acceleration(VALUE self)
{
    DeviceMotionEvent* impl = impl<DeviceMotionEvent*>(self);
    if (!impl->deviceMotionData()->acceleration())
        return Qnil;
    return createAccelerationObject(impl->deviceMotionData()->acceleration());
}

VALUE RBDeviceMotionEvent::acceleration_including_gravity(VALUE self)
{
    DeviceMotionEvent* impl = impl<DeviceMotionEvent*>(self);
    if (!impl->deviceMotionData()->acceleration())
        return Qnil;
    return createAccelerationObject(impl->deviceMotionData()->accelerationIncludingGravity());
}

VALUE RBDeviceMotionEvent::rotation_rate(VALUE self)
{
    DeviceMotionEvent* impl = impl<DeviceMotionEvent*>(self);
    if (!impl->deviceMotionData()->rotationRate())
        return Qnil;
    return createRotationRateObject(impl->deviceMotionData()->rotationRate());
}

VALUE RBDeviceMotionEvent::interval(VALUE self)
{
    DeviceMotionEvent* impl = impl<DeviceMotionEvent*>(self);
    if (!impl->deviceMotionData()->canProvideInterval())
        return Qnil;
    return toRB(impl->deviceMotionData()->interval());
}

VALUE RBDeviceMotionEvent::init_device_motion_event(VALUE self, VALUE rbType, VALUE rbBubbles, VALUE rbCancelable, VALUE rbAcceleration, VALUE rbAccelerationIncludingGravity, VALUE rbRotationRate, VALUE rbInterval)
{
    // FIXME: This should be a static/class method in Ruby, should it not?

    const String type = StringValueCStr(rbType);
    bool bubbles = RTEST(rbBubbles);
    bool cancelable = RTEST(rbCancelable);

    RefPtr<DeviceMotionData::Acceleration> acceleration = readAccelerationArgument(rbAcceleration);
    RefPtr<DeviceMotionData::Acceleration> accelerationIncludingGravity = readAccelerationArgument(rbAccelerationIncludingGravity);
    RefPtr<DeviceMotionData::RotationRate> rotationRate = readRotationRateArgument(rbRotationRate);

    bool intervalProvided = !NIL_P(rbInterval);
    double interval = NUM2DBL(rbInterval);
    RefPtr<DeviceMotionData> deviceMotionData = DeviceMotionData::create(acceleration, accelerationIncludingGravity, rotationRate, intervalProvided, interval);
    DeviceMotionEvent* impl = impl<DeviceMotionEvent>(self);
    impl->initDeviceMotionEvent(type, bubbles, cancelable, deviceMotionData.get());
    return Qnil;
}
    
} // namespace WebCore

#endif // ENABLE(DEVICE_ORIENTATION)
