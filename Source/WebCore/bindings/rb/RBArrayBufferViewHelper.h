/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef RBArrayBufferViewHelper_h
#define RBArrayBufferViewHelper_h

#include "JSArrayBufferViewHelper.h"
#include "RBArrayBuffer.h"
#include "RBArrayBufferView.h"
#include "RBConverters.h"
#include <wtf/ArrayBufferView.h>

namespace WebCore {

// Template function used by XXXArrayConstructors.
// If this returns 0, it will already have thrown an exception.
template<class C, typename T>
PassRefPtr<C> createArrayBufferViewWithTypedArrayArgument(VALUE rbArrayBufferView)
{    
    RefPtr<ArrayBufferView> source = impl<ArrayBufferView>(rbArrayBufferView);
    if (!source)
        return 0;

    ArrayBufferView::ViewType sourceType = source->getType();
    if (sourceType == ArrayBufferView::TypeDataView)
        return 0;

    uint32_t length = NUM2UINT(rb_funcall(rbArrayBufferView, rb_intern("length"), 0));
    RefPtr<C> array = C::createUninitialized(length);
    if (!array) {
        rb_raise(rb_eRangeError, "Size is too large (or is negative).");
        return array;
    }

    if (!(copyTypedArrayBuffer<C, T>(array.get(), source.get(), length, 0))) {
        rb_raise(rb_eRangeError, "Size is too large (or is negative).");
        return array;
    }

    return array;
}

// Template function used by XXXArrayConstructors.
// If this returns 0, it will already have thrown an exception.
template<class C, typename T>
PassRefPtr<C> createArrayBufferViewWithArrayBufferArgument(VALUE rbBuffer, VALUE rbByteOffset, VALUE rbLength)
{
    // This variable (tooLargeSize) is included from JSArrayBufferViewHelper.h
    // We don't use it here, but the compiler complains about it.
    // We just need to put it somewhere.
    UNUSED_PARAM(tooLargeSize);
    
    RefPtr<ArrayBuffer> buffer = impl<ArrayBuffer>(rbBuffer);
    if (!buffer)
        return 0;

    unsigned offset = NIL_P(rbByteOffset) ? 0 : NUM2UINT(rbByteOffset);
    unsigned int length = 0;
    if (!NIL_P(rbLength)) {
        length = NUM2UINT(rbLength);
    } else {
        if ((buffer->byteLength() - offset) % sizeof(T)) {
            rb_raise(rb_eRangeError, "ArrayBuffer length minus the byteOffset is not a multiple of the element size.");
            return 0;
        }
        length = (buffer->byteLength() - offset) / sizeof(T);
    }

    RefPtr<C> array = C::create(buffer, offset, length);
    if (!array)
        rb_raise(rb_eRangeError, "Size is too large (or is negative).");
    return array;
}

template<class C, typename T>
PassRefPtr<C> createArrayBufferView(int argc, VALUE* argv)
{
    // There are 3 constructors:
    //
    //  1) (in int size)
    //  2) (in ArrayBuffer buffer, [Optional] in int offset, [Optional] in unsigned int length)
    //  3) (in sequence<T>) - This ends up being an "array-like" object
    //    

    VALUE arg0, arg1, arg2;
    int argCount = rb_scan_args(argc, argv, "03", &arg0, &arg1, &arg2);

    // For the 0 args case, just create a zero-length view.
    if (argCount < 1)
        return C::create(0);
    
    if (NIL_P(arg0)) {
        rb_raise(rb_eArgError, "The first argument of a TypedArray constructor must not be nil.");
        return 0;
    }

    if (IS_RB_NUM(arg0)) {
        long length = NUM2LONG(arg0);
        RefPtr<C> result;
        if (length >= 0)
            result = C::create(length);
        if (!result)
            rb_raise(rb_eRangeError, "Size is too large (or is negative).");
        return result;
    }

    if (IS_RB_KIND(arg0, ArrayBuffer))
        return createArrayBufferViewWithArrayBufferArgument<C, T>(arg0, arg1, arg2);

    if (IS_RB_KIND(arg0, ArrayBufferView))
        return createArrayBufferViewWithTypedArrayArgument<C, T>(arg0);

    if (TYPE(arg0) == T_ARRAY) {
        uint32_t length = RARRAY_LEN(arg0);
        RefPtr<C> array = C::createUninitialized(length);
        if (!array) {
            rb_raise(rb_eRangeError, "Size is too large (or is negative).");
            return array;
        }

        for (unsigned i = 0; i < length; ++i) {
            VALUE v = rb_ary_entry(arg0, i);
            if (IS_RB_FLOAT(v))
                array->set(i, static_cast<T>(NUM2DBL(v)));
            else
                array->set(i, static_cast<T>(NUM2LL(v)));
        }

        return array;
    }
    
    return 0;
}

template<class C>
void setTypedArrayHelper(int argc, VALUE* argv, C* impl)
{
    VALUE rbArray, rbOffset;
    rb_scan_args(argc, argv, "11", &rbArray, &rbOffset);

    if (NIL_P(rbArray))
        return;
    
    if (!IS_RB_KIND(rbArray, ArrayBufferView) && !IS_RB_ARRAY(rbArray))
        rb_raise(rb_eArgError, "Invalid argument");

    uint32_t offset = 0;
    if (!NIL_P(rbOffset))
        offset = NUM2INT(rbOffset);

    VALUE rbLength = rb_funcall(rbArray, rb_intern("length"), 0);
    uint32_t length = NUM2INT(rbLength);
    if (!impl->checkInboundData(offset, length)) {
        rb_raise(rb_eRangeError, "Index is out of range.");
    } else {
        for (uint32_t i = 0; i < length; i++) {
            VALUE v = rb_funcall(rbArray, rb_intern("[]"), 1, INT2NUM(i));
            impl->set(i + offset, NUM2DBL(v));
        }
    }
}

} // namespace WebCore

#endif // RBArrayBufferViewHelper_h
