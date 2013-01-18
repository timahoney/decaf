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
#include "RBMessagePortCustom.h"

#include "RBArrayBuffer.h"
#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include "RBMessagePort.h"
#include <Ruby/ruby.h>

namespace WebCore {

VALUE RBMessagePort::post_message(int argc, VALUE* argv, VALUE self)
{
    MessagePort* selfImpl = impl<MessagePort>(self);
    return handlePostMessage(argc, argv, selfImpl);
}

void fillMessagePortArray(VALUE sequence, MessagePortArray& portArray, ArrayBufferArray& arrayBuffers)
{
    // Convert from the passed-in JS array-like object to a MessagePortArray.
    // Also validates the elements per sections 4.1.13 and 4.1.15 of the WebIDL spec and section 8.3.3 of the HTML5 spec.
    if (NIL_P(sequence)) {
        portArray.resize(0);
        arrayBuffers.resize(0);
        return;
    }

    // Validation of sequence types, per WebIDL spec 4.1.13.
    if (!IS_RB_ARRAY(sequence))
        return;

    // FIXME: Should the transferrable indices be set 
    // in SerializedScriptValue? It seems confusing to put it here, 
    // but we don't have access to this Ruby array in SerializedScriptValue.

    unsigned length = RARRAY_LEN(sequence);
    for (unsigned i = 0 ; i < length; ++i) {
        VALUE value = rb_ary_entry(sequence, i);

        // Validation of non-null objects, per HTML5 spec 10.3.3.
        if (NIL_P(value)) {
            rbDOMRaiseError(INVALID_STATE_ERR);
            return;
        }

        // Validation of Objects implementing an interface, per WebIDL spec 4.1.15.
        if (IS_RB_KIND(value, MessagePort)) {
            RefPtr<MessagePort> port = impl<MessagePort>(value);

            // Check for duplicate ports.
            if (portArray.contains(port)) {
                rbDOMRaiseError(INVALID_STATE_ERR);
                return;
            }
            portArray.append(port.release());

        } else if (IS_RB_KIND(value, ArrayBuffer)) {
            RefPtr<ArrayBuffer> arrayBuffer = impl<ArrayBuffer>(value);
            arrayBuffers.append(arrayBuffer);
        } else {
            rb_raise(rb_eTypeError, "Expected MessagePort or ArrayBuffer");
            return;
        }
    }
}

VALUE RBMessagePortCustom::marshal_load(VALUE, VALUE)
{
    // FIXME: Do we need to implement non-transferable MessagePorts?
    return Qnil;
}

VALUE RBMessagePortCustom::marshal_dump(VALUE, VALUE)
{
    // FIXME: Do we need to implement non-transferable MessagePorts?
    return rb_str_new2("");
}

void RBMessagePortCustom::Init_MessagePortCustom()
{
    rb_define_method(RBMessagePort::rubyClass(), "_dump", RUBY_METHOD_FUNC(&RBMessagePortCustom::marshal_dump), 1);
    rb_define_module_function(RBMessagePort::rubyClass(), "_load", RUBY_METHOD_FUNC(&RBMessagePortCustom::marshal_load), 1);
}

} // namespace WebCore
