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
#include "RBPopStateEvent.h"

#include "RBHistory.h"
#include "RBScriptValue.h"

namespace WebCore {

VALUE RBPopStateEvent::state_getter(VALUE self)
{
    PopStateEvent* event = impl<PopStateEvent>(self);

    // FIXME: This cached state will only do something when we cache Ruby wrappers.
    VALUE cachedValue = rb_iv_get(self, "@state");
    if (!NIL_P(cachedValue))
        return cachedValue;

    if (!event->state().hasNoValue()) {
        VALUE value = static_cast<RBScriptValue*>(event->state().delegate())->rbValue();
        rb_iv_set(self, "@state", value);
        return value;
    }

    History* history = event->history();
    if (!history || !event->serializedState())
        return Qnil;

    // There's no cached value from a previous invocation, nor a state value was provided by the
    // event, but there is a history object, so first we need to see if the state object has been
    // deserialized through the history object already.
    // The current history state object might've changed in the meantime, so we need to take care
    // of using the correct one, and always share the same deserialization with history.state.

    bool isSameState = history->isSameAsCurrentState(event->serializedState().get());
    VALUE result;

    if (isSameState) {
        VALUE rbHistory = toRB(history);
        result = RBHistory::state_getter(rbHistory);
    } else {
        result = event->serializedState()->deserializeRB();
    }

    rb_iv_set(self, "@state", result);
    return result;
}


} // namespace WebCore
