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
#include "RBImageData.h"

#include "RBConverters.h"
#include "RBImageDataCustom.h"
#include "RBObject.h"
#include "RBUint8ClampedArray.h"

namespace WebCore {

extern "C" VALUE imagedata_get_data(VALUE self);
VALUE imagedata_get_data(VALUE self)
{
    ImageData* selfImpl = impl<ImageData>(self);
    Uint8ClampedArray* data = selfImpl->data();
    return toRB(data);
}

VALUE toRB(PassRefPtr<ImageData> impl)
{
    if (!impl)
        return Qnil;

    // FIXME: The JS bindings cache the data array directly on the JS object.
    // We can't do this at the moment because Ruby's Marshal module
    // will try to serialize it. Instead, just define the 'data' getter.
    static bool didDefineGetter = false;
    if (!didDefineGetter) {
        didDefineGetter = true;
        rb_define_method(RBImageData::rubyClass(), "data", RUBY_METHOD_FUNC(&imagedata_get_data), 0);
    }
    
    return toRB(RBImageData::rubyClass(), impl);
}

VALUE RBImageDataCustom::marshal_load(VALUE, VALUE data)
{
    UNUSED_PARAM(data);
    
    // FIXME: This doesn't work. The image data is corrupted.
    
    // VALUE divider = rb_str_new2("*");
    // VALUE dataArray = rb_funcall(data, rb_intern("split"), 1, divider);
    
    // VALUE widthString = rb_ary_shift(dataArray);
    // VALUE widthRB = rb_funcall(widthString, rb_intern("to_i"), 0);
    // int width = NUM2INT(widthRB);
    // VALUE heightString = rb_ary_shift(dataArray);
    // VALUE heightRB = rb_funcall(heightString, rb_intern("to_i"), 0);
    // int height = NUM2INT(heightRB);
    // RefPtr<ImageData> result = ImageData::create(IntSize(width, height));
    
    // VALUE lengthString = rb_ary_shift(dataArray);
    // VALUE lengthRB = rb_funcall(lengthString, rb_intern("to_i"), 0);
    // unsigned length = NUM2UINT(lengthRB);
    // VALUE restString = rb_ary_shift(dataArray);
    // memcpy(result->data()->data(), RSTRING_PTR(restString), length);
    
    // return toRB(result.get());

    return Qnil;
}

VALUE RBImageDataCustom::marshal_dump(VALUE self, VALUE)
{
    UNUSED_PARAM(self);
    
    // FIXME: This doesn't work. The data from imageDataString is corrupt after loading again.
    
    // ImageData* imageData = impl<ImageData>(self);
    // VALUE dumpString = rb_str_new2("");
    // VALUE divider = rb_str_new2("*");
    // VALUE widthString = rb_funcall(INT2FIX(imageData->width()), rb_intern("to_s"), 0);
    // rb_str_append(dumpString, widthString);
    // rb_str_append(dumpString, divider);
    // VALUE heightString = rb_funcall(INT2FIX(imageData->height()), rb_intern("to_s"), 0);
    // rb_str_append(dumpString, heightString);
    // rb_str_append(dumpString, divider);
    
    // unsigned length = imageData->data()->length();
    // VALUE lengthString = rb_funcall(UINT2NUM(length), rb_intern("to_s"), 0);
    // rb_str_append(dumpString, lengthString);
    // rb_str_append(dumpString, divider);
    // VALUE imageDataString = rb_str_new(buffer, length);
    // rb_str_append(dumpString, imageDataString);
    // return dumpString;
    
    rb_raise(rb_eArgError, "ImageData is not yet supported for serialized message data. Try passing its 'data' buffer instead.");
    
    return Qnil;
}

void RBImageDataCustom::Init_ImageDataCustom()
{
    rb_define_method(RBImageData::rubyClass(), "_dump", RUBY_METHOD_FUNC(&RBImageDataCustom::marshal_dump), 1);
    rb_define_module_function(RBImageData::rubyClass(), "_load", RUBY_METHOD_FUNC(&RBImageDataCustom::marshal_load), 1);
}

} // namespace WebCore
