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

#ifndef RBDOMBindingAttributes_h
#define RBDOMBindingAttributes_h

#include <Ruby/ruby.h>

namespace WebCore {

class RBDOMBindingAttributes {
public:

    static VALUE dom_binding_attributes_getter(VALUE self);
    static VALUE add_dom_binding_attribute(VALUE self, VALUE attributeName);

    static void Init_DOMBindingAttributes() {
        static bool defined = false;
        if (defined)
            return;
        defined = true;

        rb_mDOMDOMBindingAttributes = rb_define_module("DOMBindingAttributes");
        rb_define_method(rb_mDOMDOMBindingAttributes, "dom_binding_attributes", RUBY_METHOD_FUNC(&RBDOMBindingAttributes::dom_binding_attributes_getter), 0);
        rb_define_method(rb_mDOMDOMBindingAttributes, "add_dom_binding_attribute", RUBY_METHOD_FUNC(&RBDOMBindingAttributes::add_dom_binding_attribute), 1);
    }

    static VALUE rubyClass() {
        Init_DOMBindingAttributes();
        return rb_mDOMDOMBindingAttributes;
    }

private:
    static VALUE rb_mDOMDOMBindingAttributes;
};

} // namespace WebCore

#endif
