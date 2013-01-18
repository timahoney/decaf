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
#include "RBMessageEvent.h"

#include "RBArrayBuffer.h"
#include "RBBlob.h"
#include "RBConverters.h"
#include "RBDOMWindow.h"
#include "RBMessagePortCustom.h"
#include "RBScriptValue.h"
#include "ScriptValue.h"
#include "SerializedScriptValue.h"
#include <Ruby/ruby.h>

namespace WebCore {

VALUE RBMessageEvent::data_getter(VALUE self)
{
    VALUE cachedValue = rb_iv_get(self, "@data");
    if (!NIL_P(cachedValue))
        return cachedValue;

    MessageEvent* event = impl<MessageEvent>(self);
    VALUE result;
    switch (event->dataType()) {
    case MessageEvent::DataTypeScriptValue: {
        ScriptValue scriptValue = event->dataAsScriptValue();
        if (scriptValue.hasNoValue() || scriptValue.scriptType() != RBScriptType)
            result = Qnil;
        else
            result = static_cast<RBScriptValue*>(scriptValue.delegate())->rbValue();
        break;
    }

    case MessageEvent::DataTypeSerializedScriptValue:
        if (RefPtr<SerializedScriptValue> serializedValue = event->dataAsSerializedScriptValue())
            result = serializedValue->deserializeRB(event->ports());
        else
            result = Qnil;
        break;

    case MessageEvent::DataTypeString:
        result = toRB(event->dataAsString());
        break;

    case MessageEvent::DataTypeBlob:
        result = toRB(event->dataAsBlob());
        break;

    case MessageEvent::DataTypeArrayBuffer:
        result = toRB(event->dataAsArrayBuffer());
        break;
    }

    // Save the result so we don't have to deserialize the value again.
    rb_iv_set(self, "@data", result);
    return result;
}

VALUE RBMessageEvent::ports_getter(VALUE self)
{
    MessageEvent* selfImpl = impl<MessageEvent>(self);
    MessagePortArray* ports = selfImpl->ports();
    if (!ports)
        return rb_ary_new();

    return toRB(ports);
}

static VALUE handleInitMessageEvent(int argc, VALUE* argv, VALUE self)
{
    VALUE typeRB, canBubbleRB, cancelableRB, dataRB, originRB, lastEventIdRB, sourceRB, portsRB;
    rb_scan_args(argc, argv, "08", &typeRB, &canBubbleRB, &cancelableRB, &dataRB, &originRB, &lastEventIdRB, &sourceRB, &portsRB);

    const String& typeArg = rbToString(typeRB);
    bool canBubbleArg = RTEST(canBubbleRB);
    bool cancelableArg = RTEST(cancelableRB);
    ScriptValue dataArg = RBScriptValue::scriptValue(dataRB);
    const String originArg = rbToString(originRB);
    const String lastEventIdArg = rbToString(lastEventIdRB);
    DOMWindow* sourceArg = impl<DOMWindow>(sourceRB);
    OwnPtr<MessagePortArray> messagePorts;
    OwnPtr<ArrayBufferArray> arrayBuffers;
    if (!NIL_P(portsRB)) {
        messagePorts = adoptPtr(new MessagePortArray);
        arrayBuffers = adoptPtr(new ArrayBufferArray);
        fillMessagePortArray(portsRB, *messagePorts, *arrayBuffers);
        if (!NIL_P(rb_errinfo()))
            return Qnil;
    }

    MessageEvent* event = impl<MessageEvent>(self);
    event->initMessageEvent(typeArg, canBubbleArg, cancelableArg, dataArg, originArg, lastEventIdArg, sourceArg, messagePorts.release());
    rb_iv_set(self, "@data", dataRB);
    return Qnil;
}

VALUE RBMessageEvent::init_message_event(int argc, VALUE* argv, VALUE self)
{
    return handleInitMessageEvent(argc, argv, self);
}

VALUE RBMessageEvent::webkit_init_message_event(int argc, VALUE* argv, VALUE self)
{
    return handleInitMessageEvent(argc, argv, self);
}

} // namespace WebCore
