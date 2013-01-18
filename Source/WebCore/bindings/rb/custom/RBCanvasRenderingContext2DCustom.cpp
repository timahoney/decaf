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
#include "RBCanvasRenderingContext2D.h"

#include "CanvasStyle.h"
#include "RBCanvasGradient.h"
#include "RBCanvasPattern.h"
#include "RBConverters.h"
#include <Ruby/ruby.h>
#include <wtf/Vector.h>

namespace WebCore {

static VALUE toRB(CanvasStyle* style)
{
    if (style->canvasGradient())
        return toRB(style->canvasGradient());
    if (style->canvasPattern())
        return toRB(style->canvasPattern());
    return toRB(style->color());
}

inline PassRefPtr<CanvasStyle> toCanvasStyle(VALUE style)
{
    if (IS_RB_KIND(style, CanvasGradient))
        return CanvasStyle::createFromGradient(impl<CanvasGradient>(style));
    if (IS_RB_KIND(style, CanvasPattern))
        return CanvasStyle::createFromPattern(impl<CanvasPattern>(style));
    return 0;
}

VALUE RBCanvasRenderingContext2D::webkit_line_dash_getter(VALUE self)
{
    CanvasRenderingContext2D* selfImpl = impl<CanvasRenderingContext2D>(self);
    Vector<float> dash = selfImpl->getLineDash();
    VALUE array = rb_ary_new2(dash.size());
    Vector<float>::const_iterator end = dash.end();
    for (Vector<float>::const_iterator it = dash.begin(); it != end; ++it)
        rb_ary_push(array, toRB(*it));

    return array;
}

VALUE RBCanvasRenderingContext2D::webkit_line_dash_setter(VALUE self, VALUE newWebkitLineDash)
{
    if (!IS_RB_ARRAY(newWebkitLineDash))
        return Qnil;

    Vector<float> dash;
    unsigned length = FIX2INT(rb_funcall(newWebkitLineDash, rb_intern("length"), 0));
    for (unsigned i = 0; i < length; ++i) {
        float elem = NUM2DBL(rb_ary_entry(newWebkitLineDash, i));
        if (elem <= 0)
            return Qnil;
        dash.append(elem);
    }

    CanvasRenderingContext2D* selfImpl = impl<CanvasRenderingContext2D>(self);
    selfImpl->setWebkitLineDash(dash);
    return newWebkitLineDash;
}

VALUE RBCanvasRenderingContext2D::stroke_style_getter(VALUE self)
{
    CanvasRenderingContext2D* selfImpl = impl<CanvasRenderingContext2D>(self);
    CanvasStyle* style = selfImpl->strokeStyle();
    return toRB(style);
}

VALUE RBCanvasRenderingContext2D::stroke_style_setter(VALUE self, VALUE newStrokeStyle)
{
    CanvasRenderingContext2D* selfImpl = impl<CanvasRenderingContext2D>(self);
    if (IS_RB_STRING(newStrokeStyle)) {
        String colorString = StringValueCStr(newStrokeStyle);
        selfImpl->setStrokeColor(colorString);
    } else {
        RefPtr<CanvasStyle> style = toCanvasStyle(newStrokeStyle);
        selfImpl->setStrokeStyle(style.release());
    }
    return newStrokeStyle;
}

VALUE RBCanvasRenderingContext2D::fill_style_getter(VALUE self)
{
    CanvasRenderingContext2D* selfImpl = impl<CanvasRenderingContext2D>(self);
    CanvasStyle* style = selfImpl->fillStyle();
    return toRB(style);
}

VALUE RBCanvasRenderingContext2D::fill_style_setter(VALUE self, VALUE newFillStyle)
{
    CanvasRenderingContext2D* selfImpl = impl<CanvasRenderingContext2D>(self);
    if (IS_RB_STRING(newFillStyle)) {
        String colorString = StringValueCStr(newFillStyle);
        selfImpl->setFillColor(colorString);
    } else {
        RefPtr<CanvasStyle> style = toCanvasStyle(newFillStyle);
        selfImpl->setFillStyle(style.release());
    }
    return newFillStyle;
}

} // namespace WebCore
