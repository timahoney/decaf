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
#include "RBDataViewCustom.h"

#include "RBArrayBuffer.h"
#include "RBArrayBufferViewHelper.h"
#include "RBConverters.h"
#include "RBExceptionHandler.h"

namespace WebCore {

VALUE RBDataView::rb_new(int argc, VALUE* argv, VALUE)
{
    VALUE buffer, byteOffset, byteLength;
    rb_scan_args(argc, argv, "12", &buffer, &byteOffset, &byteLength);

    if (!IS_RB_KIND(buffer, ArrayBuffer)) {
        rb_raise(rb_eTypeError, "The first argument to the DataView constructor must be an ArrayBuffer");
        return Qnil;
    }

    RefPtr<DataView> view = createArrayBufferViewWithArrayBufferArgument<DataView, char>(buffer, byteOffset, byteLength);
    if (!view.get())
        return Qnil;

    return toRB(view.release());
}

VALUE RBDataViewCustom::get_int8(VALUE self, VALUE byteOffset)
{
    DataView* selfImpl = impl<DataView>(self);
    ExceptionCode ec = 0;
    unsigned long byteOffsetImpl = NUM2ULONG(byteOffset);
    int8_t result = selfImpl->getInt8(byteOffsetImpl, ec);
    RB::setDOMException(ec);
    return toRB(result);
}

VALUE RBDataViewCustom::get_uint8(VALUE self, VALUE byteOffset)
{
    DataView* selfImpl = impl<DataView>(self);
    ExceptionCode ec = 0;
    unsigned long byteOffsetImpl = NUM2ULONG(byteOffset);
    uint8_t result = selfImpl->getUint8(byteOffsetImpl, ec);
    RB::setDOMException(ec);
    return toRB(result);
}

VALUE RBDataViewCustom::set_int8(VALUE self, VALUE byteOffset, VALUE value)
{
    DataView* selfImpl = impl<DataView>(self);
    ExceptionCode ec = 0;
    unsigned long byteOffsetImpl = NUM2ULONG(byteOffset);
    int8_t valueImpl = NUM2INT(value);
    selfImpl->setInt8(byteOffsetImpl, valueImpl, ec);
    RB::setDOMException(ec);
    return Qnil;
}

VALUE RBDataViewCustom::set_uint8(VALUE self, VALUE byteOffset, VALUE value)
{
    DataView* selfImpl = impl<DataView>(self);
    ExceptionCode ec = 0;
    unsigned long byteOffsetImpl = NUM2ULONG(byteOffset);
    uint8_t valueImpl = NUM2UINT(value);
    selfImpl->setUint8(byteOffsetImpl, valueImpl, ec);
    RB::setDOMException(ec);
    return Qnil;
}

VALUE toRB(DataView* impl)
{
    // The JS custom implementation of this is nothing special.
    // FIXME: Should the IDL for DataView not have CustomToJSObject?
    return toRB(RBDataView::rubyClass(), impl);
}

// FIXME: None of these will ever get called.
//        For some reason, the custom IDL declarations for these functions take no arguments.
//        The actual functions should take arguments.
//        It's alright for JS because their arguments don't have to be specified explicitly.
//        At some point, the IDL needs to be changed.
RB_UNIMPLEMENTED(RBDataView::get_int8(VALUE))
RB_UNIMPLEMENTED(RBDataView::get_uint8(VALUE))
RB_UNIMPLEMENTED(RBDataView::set_int8(VALUE))
RB_UNIMPLEMENTED(RBDataView::set_uint8(VALUE))

} // namespace WebCore
