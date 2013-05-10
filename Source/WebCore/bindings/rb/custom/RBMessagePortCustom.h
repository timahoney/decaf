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

#ifndef RBMessagePortCustom_h
#define RBMessagePortCustom_h

#include "MessagePort.h"
#include "RBExceptionHandler.h"
#include "RBScriptValue.h"
#include "RBSerializationDelegate.h"
#include <Ruby/ruby.h>
#include <wtf/Forward.h>

namespace WebCore {

// Helper function which pulls the values out of an RB sequence and into a MessagePortArray.
void fillMessagePortArray(VALUE sequence, MessagePortArray& portArray, ArrayBufferArray& arrayBuffers);

// Helper function to convert from RB postMessage arguments to WebCore postMessage arguments.
template <typename T>
inline VALUE handlePostMessage(int argc, VALUE* argv, T* impl)
{
    VALUE rbMessage, rbTransferables;
    rb_scan_args(argc, argv, "11", &rbMessage, &rbTransferables);

    MessagePortArray portArray;
    ArrayBufferArray arrayBufferArray;
    fillMessagePortArray(rbTransferables, portArray, arrayBufferArray);
    RefPtr<SerializedScriptValue> message = SerializedScriptValue::create(RBSerializationDelegate::create(),
                                                                          RBScriptValue::scriptValue(rbMessage),
                                                                          &portArray, &arrayBufferArray);
    if (!NIL_P(rb_errinfo()))
        return Qnil;

    ExceptionCode ec = 0;
    impl->postMessage(message.release(), &portArray, ec);
    RB::setDOMException(ec);
    return Qnil;
}

} // namespace WebCore

#endif // RBMessagePortCustom_h
