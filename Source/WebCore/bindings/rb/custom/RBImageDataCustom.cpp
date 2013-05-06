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

VALUE toRB(ImageData* impl)
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

} // namespace WebCore
