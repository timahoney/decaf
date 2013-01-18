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
#include "RBArrayBuffer.h"

#include "RBArrayBufferCustom.h"
#include "RBConverters.h"

namespace WebCore {

VALUE RBArrayBufferCustom::marshal_load(VALUE, VALUE data)
{
    // FIXME: Support non-transferable ArrayBuffers.
    VALUE byteLengthRB = rb_funcall(data, rb_intern("to_i"), 0);
    unsigned byteLength = NUM2UINT(byteLengthRB);
    unsigned numElements = 0;
    RefPtr<ArrayBuffer> buffer = ArrayBuffer::create(numElements, byteLength);
    return toRB(buffer.release());
}

VALUE RBArrayBufferCustom::marshal_dump(VALUE self, VALUE)
{
    // FIXME: Support non-transferable ArrayBuffers.
    ArrayBuffer* buffer = impl<ArrayBuffer>(self);
    return toRB(String::number(buffer->byteLength()));
}

void RBArrayBufferCustom::Init_ArrayBufferCustom()
{
    rb_define_method(RBArrayBuffer::rubyClass(), "_dump", RUBY_METHOD_FUNC(&RBArrayBufferCustom::marshal_dump), 1);
    rb_define_module_function(RBArrayBuffer::rubyClass(), "_load", RUBY_METHOD_FUNC(&RBArrayBufferCustom::marshal_load), 1);
}

VALUE RBArrayBuffer::rb_new(int argc, VALUE* argv, VALUE)
{
    VALUE rbLength;
    rb_scan_args(argc, argv, "01", &rbLength);

    int length = 0;
    if (!NIL_P(rbLength))
        length = NUM2INT(rbLength);
    RefPtr<ArrayBuffer> buffer;
    if (length >= 0)
        buffer = ArrayBuffer::create(static_cast<unsigned>(length), 1);

    if (!buffer.get()) {
        rb_raise(rb_eArgError, "ArrayBuffer size is not a small enough positive integer.");
        return Qnil;
    }
    
    return toRB(buffer.get());
}
    
} // namespace WebCore
