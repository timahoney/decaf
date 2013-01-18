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
#include "RBHTMLInputElement.h"

#include "RBConverters.h"

namespace WebCore {

static VALUE throwInputTypeError()
{
    rb_raise(rb_eTypeError, "This input element cannot have selection.");
    return Qnil;
}

VALUE RBHTMLInputElement::selection_start_getter(VALUE self)
{
    HTMLInputElement* input = impl<HTMLInputElement>(self);
    if (!input->canHaveSelection())
        return throwInputTypeError();

    return toRB(input->selectionStart());
}

VALUE RBHTMLInputElement::selection_start_setter(VALUE self, VALUE newSelectionStart)
{
    HTMLInputElement* input = impl<HTMLInputElement>(self);
    if (!input->canHaveSelection())
        return throwInputTypeError();

    input->setSelectionStart(NUM2INT(newSelectionStart));
    return newSelectionStart;
}

VALUE RBHTMLInputElement::selection_end_getter(VALUE self)
{
    HTMLInputElement* input = impl<HTMLInputElement>(self);
    if (!input->canHaveSelection())
        return throwInputTypeError();

    return toRB(input->selectionEnd());
}

VALUE RBHTMLInputElement::selection_end_setter(VALUE self, VALUE newSelectionEnd)
{
    HTMLInputElement* input = impl<HTMLInputElement>(self);
    if (!input->canHaveSelection())
        return throwInputTypeError();

    input->setSelectionEnd(NUM2INT(newSelectionEnd));
    return newSelectionEnd;
}

VALUE RBHTMLInputElement::selection_direction_getter(VALUE self)
{
    HTMLInputElement* input = impl<HTMLInputElement>(self);
    if (!input->canHaveSelection())
        return throwInputTypeError();

    return toRB(input->selectionDirection());
}

VALUE RBHTMLInputElement::selection_direction_setter(VALUE self, VALUE newSelectionDirection)
{
    HTMLInputElement* input = impl<HTMLInputElement>(self);
    if (!input->canHaveSelection())
        return throwInputTypeError();

    input->setSelectionDirection(StringValueCStr(newSelectionDirection));
    return newSelectionDirection;
}

VALUE RBHTMLInputElement::set_selection_range(int argc, VALUE* argv, VALUE self)
{
    VALUE rbStart, rbEnd, rbDirection;
    rb_scan_args(argc, argv, "03", &rbStart, &rbEnd, &rbDirection);

    HTMLInputElement* input = impl<HTMLInputElement>(self);
    if (!input->canHaveSelection())
        return throwInputTypeError();

    int start = NIL_P(rbStart) ? 0 : NUM2INT(rbStart);
    int end = NIL_P(rbEnd) ? 0 : NUM2INT(rbEnd);
    if (NIL_P(rbDirection))
        input->setSelectionRange(start, end);
    else
        input->setSelectionRange(start, end, StringValueCStr(rbDirection));

    return Qnil;
}

} // namespace WebCore
