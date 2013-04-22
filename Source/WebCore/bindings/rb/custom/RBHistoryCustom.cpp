/*
 *  Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "RBHistory.h"

#include "BindingSecurity.h"
#include "RBConverters.h"
#include "RBExceptionHandler.h"

namespace WebCore {

VALUE RBHistory::state_getter(VALUE self)
{
    History* history = impl<History>(self);

    // FIXME: This cached value probably won't do much good until we have cached wrappers.
    // So, we should implement cached wrappers for Ruby.
    VALUE cachedValue = rb_iv_get(self, "@state");
    if (!NIL_P(cachedValue) && !history->stateChanged())
        return cachedValue;

    RefPtr<SerializedScriptValue> serialized = history->state();
    VALUE result = serialized ? serialized->deserializeRB() : Qnil;
    rb_iv_set(self, "@state", result);
    return result;
}

VALUE RBHistory::push_state(int argc, VALUE* argv, VALUE self)
{
    History *selfImpl = impl<History>(self);
    VALUE data, titleRB, urlRB;
    rb_scan_args(argc, argv, "21", &data, &titleRB, &urlRB);

    RefPtr<SerializedScriptValue> historyState = SerializedScriptValue::create(data);
    String title = rbStringOrNullString(titleRB);
    String url = rbStringOrNullString(urlRB);
    ExceptionCode ec = 0;
    selfImpl->stateObjectAdded(historyState.release(), title, url, History::StateObjectPush, ec);
    RB::setDOMException(ec);

    return Qnil;
}

VALUE RBHistory::replace_state(int argc, VALUE* argv, VALUE self)
{
    History *selfImpl = impl<History>(self);
    VALUE data, titleRB, urlRB;
    rb_scan_args(argc, argv, "21", &data, &titleRB, &urlRB);
    
    RefPtr<SerializedScriptValue> historyState = SerializedScriptValue::create(data);
    String title = rbStringOrNullString(titleRB);
    String url = rbStringOrNullString(urlRB);
    ExceptionCode ec = 0;
    selfImpl->stateObjectAdded(historyState.release(), title, url, History::StateObjectReplace, ec);
    RB::setDOMException(ec);

    return Qnil;
}

// FIXME: What is supposed to go here? Is there actually a named setter for History?
RB_UNIMPLEMENTED(RBHistory::named_setter(VALUE, VALUE, VALUE))

} // namespace WebCore
