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
#include "RBDOMFormData.h"

#include "RBBlob.h"
#include "RBConverters.h"

namespace WebCore {

VALUE RBDOMFormData::rb_new(int argc, VALUE* argv, VALUE)
{
    VALUE rbForm;
    rb_scan_args(argc, argv, "01", &rbForm);

    HTMLFormElement* form = 0;
    if (!NIL_P(rbForm))
        form = impl<HTMLFormElement>(rbForm);
    return toRB(DOMFormData::create(form));
}

VALUE RBDOMFormData::append(int argc, VALUE* argv, VALUE self)
{
    VALUE rbName, rbValue, rbFilename;
    rb_scan_args(argc, argv, "03", &rbName, &rbValue, &rbFilename);

    String name = rbStringOrUndefined(rbName);
    if (IS_RB_KIND(rbValue, Blob))
        impl<DOMFormData>(self)->append(name, impl<Blob>(rbValue), rbStringOrUndefined(rbFilename));
    else
        impl<DOMFormData>(self)->append(name, rbStringOrUndefined(rbValue));

    return Qnil;
}

} // namespace WebCore
