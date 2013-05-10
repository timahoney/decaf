/*
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

#ifndef RBObject_h
#define RBObject_h

#include <Ruby/ruby.h>
#include <wtf/RefPtr.h>

namespace WebCore {

#define RB_UNIMPLEMENTED(methodSig) \
VALUE methodSig \
{ \
    rb_notimplement(); \
    return Qnil; \
}

class RBObject {
public:    
    // Returns the class object for this class in Ruby.
    // Each subclass should implement this method.
    // This method should also ensure that the class has been instantiated.
    static VALUE rubyClass() { return rb_cObject; }
};

template <typename T>
struct RBImplWrapper {
    RefPtr<T> ptr;
};

template <typename T>
void rb_wrapper_free(RBImplWrapper<T>* wrapper)
{
    wrapper->ptr = 0;
    free(wrapper);
}

template <typename T>
VALUE toRB(VALUE klass, T* impl)
{
    if (!impl)
        return Qnil;
    
    RBImplWrapper<T>* wrapper;
    wrapper = new RBImplWrapper<T>;
    wrapper->ptr = RefPtr<T>(impl);
    VALUE instance = Data_Wrap_Struct(klass, 0, rb_wrapper_free<T>, wrapper);
    return instance;
}

template <typename T>
inline T* impl(VALUE instance)
{
    if (NIL_P(instance))
        return 0;
    
    RBImplWrapper<T>* wrapper;
    Data_Get_Struct(instance, RBImplWrapper<T>, wrapper);
    return wrapper->ptr.get();
}

} // namespace WebCore

#endif
