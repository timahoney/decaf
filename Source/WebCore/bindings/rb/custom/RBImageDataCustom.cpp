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
#include "RBUint8ClampedArray.h"

namespace WebCore {

extern "C" VALUE imagedata_get_data(VALUE self);
VALUE imagedata_get_data(VALUE self)
{
    VALUE dataWrapper = rb_iv_get(self, "@data");
    return dataWrapper;
}

VALUE toRB(PassRefPtr<ImageData> prpImpl)
{
    RefPtr<ImageData> impl = prpImpl;
    
    if (!impl)
        return Qnil;

    // The JS bindings cache the data array directly on the JS object.
    // That way, they don't always call into the C++ function to access this heavy object.
    // I guess that makes sense, so let's do it for Ruby too.

    // FIXME: This may have to change once we start caching the Ruby objects.
    // The array may have already been wrapped,
    // and the 'data' function may have already been mapped to the array.
    Uint8ClampedArray* data = impl->data();
    VALUE dataWrapper = toRB(data);
    VALUE wrapper = toRB(RBImageData::rubyClass(), impl.release());
    rb_iv_set(wrapper, "@data", dataWrapper);

    static bool didRedefineMethod = false;
    if (!didRedefineMethod) {
        didRedefineMethod = true;
        rb_undef_method(RBImageData::rubyClass(), "data");
        rb_define_method(RBImageData::rubyClass(), "data", RUBY_METHOD_FUNC(&imagedata_get_data), 0);
    }
    
    return wrapper;
}

} // namespace WebCore
