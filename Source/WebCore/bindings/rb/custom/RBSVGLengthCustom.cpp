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
#include "RBSVGLength.h"

#include "ExceptionCode.h"
#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include "SVGAnimatedProperty.h"
#include "SVGException.h"
#include "SVGLengthContext.h"

namespace WebCore {

VALUE RBSVGLength::value_getter(VALUE self)
{
    SVGPropertyTearOff<SVGLength>* selfImpl = impl<SVGPropertyTearOff<SVGLength> >(self);
    SVGLength& property = selfImpl->propertyReference();
    ExceptionCode ec = 0;
    SVGLengthContext lengthContext(selfImpl->contextElement());
    float value = property.value(lengthContext, ec);
    if (ec) {
        rbDOMRaiseError(ec);
        return Qnil;
    }

    return toRB(value);
}

VALUE RBSVGLength::value_setter(VALUE self, VALUE newValue)
{
    SVGPropertyTearOff<SVGLength>* selfImpl = impl<SVGPropertyTearOff<SVGLength> >(self);
    if (selfImpl->isReadOnly()) {
        rbDOMRaiseError(NO_MODIFICATION_ALLOWED_ERR);
        return Qnil;
    }

    if (!IS_RB_NUM(newValue) && !IS_RB_BOOL(newValue) && !NIL_P(newValue)) {
        rb_raise(rb_eTypeError, "Type error");
        return Qnil;
    }

    SVGLength& property = selfImpl->propertyReference();

    ExceptionCode ec = 0;
    SVGLengthContext lengthContext(selfImpl->contextElement());
    property.setValue(NUM2DBL(newValue), lengthContext, ec);
    if (ec) {
        rbDOMRaiseError(ec);
        return Qnil;
    }

    selfImpl->commitChange();
    return Qnil;
}

VALUE RBSVGLength::convert_to_specified_units(VALUE self, VALUE rbUnitType)
{
    SVGPropertyTearOff<SVGLength>* selfImpl = impl<SVGPropertyTearOff<SVGLength> >(self);
    if (selfImpl->isReadOnly()) {
        rbDOMRaiseError(NO_MODIFICATION_ALLOWED_ERR);
        return Qnil;
    }

    SVGLength& property = selfImpl->propertyReference();

    if (NIL_P(rbUnitType)) {
        rb_raise(rb_eArgError, "Not enough arguments");
        return Qnil;
    }

    unsigned short unitType = NUM2UINT(rbUnitType);

    ExceptionCode ec = 0;
    SVGLengthContext lengthContext(selfImpl->contextElement());
    property.convertToSpecifiedUnits(unitType, lengthContext, ec);
    if (ec) {
        rbDOMRaiseError(ec);
        return Qnil;
    }

    selfImpl->commitChange();
    return Qnil;
}

} // namespace WebCore
