/*
 *  Copyright (C) 2012 Tim Mahoney <tim.mahoney@me.com>
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
#include "RBDOMWindow.h"

#include "BindingSecurity.h"
#include "ContentSecurityPolicy.h"
#include "Location.h"
#include "RBArrayBuffer.h"
#include "RBConverters.h"
#include "RBEventListener.h"
#include "RBExceptionHandler.h"
#include "RBLocation.h"
#include "RBMessagePort.h"
#include "RBMessagePortCustom.h"
#include "RBScheduledAction.h"
#include "RBScriptState.h"
#include <wtf/Vector.h>

namespace WebCore {

VALUE RBDOMWindow::location_setter(VALUE self, VALUE newValue)
{
    DOMWindow* selfImpl = impl<DOMWindow>(self);
    RBScriptState* state = RBScriptState::current();
    if (!BindingSecurity::shouldAllowAccessToDOMWindow(state, selfImpl))
        return Qnil;

    String locationString = StringValueCStr(newValue);
    if (Location* location = selfImpl->location())
        location->setHref(locationString, activeDOMWindow(state), firstDOMWindow(state));
    
    return newValue;
}

VALUE RBDOMWindow::event_getter(VALUE self)
{
    // FIXME: Store the current event somewhere and retrieve it here.
    // This means modifying something in RBEventListener to set the current event.
    // The problem is: where do we store it?
    UNUSED_PARAM(self);
    return Qnil;
}

VALUE RBDOMWindow::open(int argc, VALUE* argv, VALUE self)
{
    VALUE url, name, options;
    rb_scan_args(argc, argv, "21", &url, &name, &options);

    String urlString = rbStringOrNullString(url);
    AtomicString frameName = NIL_P(name) ? "_blank" : StringValueCStr(name);
    String windowFeaturesString = rbStringOrNullString(options);

    DOMWindow* selfImpl = impl<DOMWindow>(self);
    RBScriptState* state = RBScriptState::current();
    RefPtr<DOMWindow> openedWindow = selfImpl->open(urlString, frameName, windowFeaturesString, activeDOMWindow(state), firstDOMWindow(state));
    return toRB(openedWindow.release());
}

VALUE RBDOMWindow::show_modal_dialog(int argc, VALUE* argv, VALUE self)
{
    VALUE url, dialogArgs, featureArgs;
    rb_scan_args(argc, argv, "12", &url, &dialogArgs, &featureArgs);
    UNUSED_PARAM(self);
    UNUSED_PARAM(url);
    UNUSED_PARAM(dialogArgs);
    UNUSED_PARAM(featureArgs);

    // FIXME: This is unsupported for now.
    // showModalDialog seems like a bad idea anyway.
    rb_notimplement();

    return Qnil;
}

static VALUE handlePostMessage(int argc, VALUE* argv, VALUE self)
{
    DOMWindow* selfImpl = impl<DOMWindow>(self);
    VALUE rbMessage, rbTargetOrigin, rbTransferables;
    rb_scan_args(argc, argv, "12", &rbMessage, &rbTargetOrigin, &rbTransferables);

    MessagePortArray messagePorts;
    ArrayBufferArray arrayBuffers;

    // This function has variable arguments and can be:
    // Per current spec:
    //   postMessage(message, targetOrigin)
    //   postMessage(message, targetOrigin, {sequence of transferrables})
    // Legacy non-standard implementations in webkit allowed:
    //   postMessage(message, {sequence of transferrables}, targetOrigin);
    if (!NIL_P(rbTransferables)) {
        if (IS_RB_STRING(rbTransferables)) {
            VALUE temp = rbTransferables;
            rbTransferables = rbTargetOrigin;
            rbTargetOrigin = temp;
        }
        fillMessagePortArray(rbTransferables, messagePorts, arrayBuffers);
    }

    RefPtr<SerializedScriptValue> message = SerializedScriptValue::create(rbMessage, &messagePorts, &arrayBuffers);
    String targetOrigin = StringValueCStr(rbTargetOrigin);
    ExceptionCode ec = 0;
    RBScriptState* state = RBScriptState::current();
    selfImpl->postMessage(message.release(), &messagePorts, targetOrigin, activeDOMWindow(state), ec);
    RB::setDOMException(ec);

    return Qnil;
}

VALUE RBDOMWindow::post_message(int argc, VALUE* argv, VALUE self)
{
    return handlePostMessage(argc, argv, self);
}

VALUE RBDOMWindow::set_timeout(int argc, VALUE* argv, VALUE self)
{
    DOMWindow* selfImpl = impl<DOMWindow>(self);
    RBScriptState* state = RBScriptState::current();
    ContentSecurityPolicy* policy = selfImpl->document() ? selfImpl->document()->contentSecurityPolicy() : 0;
    if (policy && !policy->allowEval(state))
        return INT2FIX(0);
    
    VALUE handler, timeoutRB;
    rb_scan_args(argc, argv, "02", &handler, &timeoutRB);
    if (rb_block_given_p()) {
        timeoutRB = handler;
        handler = rb_block_proc();
    }

    if (NIL_P(handler))
        return INT2FIX(0);

    int timeout = NIL_P(timeoutRB) ? 0 : NUM2INT(timeoutRB);
    OwnPtr<ScheduledAction> action = RBScheduledAction::create(handler);
    ExceptionCode ec = 0;
    int result = selfImpl->setTimeout(action.release(), timeout, ec);
    RB::setDOMException(ec);
    return INT2FIX(result);
}

VALUE RBDOMWindow::set_interval(int argc, VALUE* argv, VALUE self)
{
    DOMWindow* selfImpl = impl<DOMWindow>(self);
    RBScriptState* state = RBScriptState::current();
    ContentSecurityPolicy* policy = selfImpl->document() ? selfImpl->document()->contentSecurityPolicy() : 0;
    if (policy && !policy->allowEval(state))
        return INT2FIX(0);

    VALUE handler, timeoutRB;
    rb_scan_args(argc, argv, "02", &handler, &timeoutRB);
    if (rb_block_given_p()) {
        timeoutRB = handler;
        handler = rb_block_proc();
    }

    if (NIL_P(handler) || NIL_P(timeoutRB))
        return INT2FIX(0);

    ExceptionCode ec = 0;
    OwnPtr<ScheduledAction> action = RBScheduledAction::create(handler);
    int timeout = NUM2INT(timeoutRB);
    int result = selfImpl->setInterval(action.release(), timeout, ec);
    RB::setDOMException(ec);
    return INT2FIX(result);
}

VALUE RBDOMWindow::add_event_listener(int argc, VALUE* argv, VALUE self)
{
    DOMWindow* selfImpl = impl<DOMWindow>(self);
    VALUE typeRB, listenerRB, useCaptureRB;
    rb_scan_args(argc, argv, "12", &typeRB, &listenerRB, &useCaptureRB);
    if (rb_block_given_p()) {
        useCaptureRB = listenerRB;
        listenerRB = rb_block_proc();
    }

    String type = StringValueCStr(typeRB);
    bool useCapture = RTEST(useCaptureRB);
    RefPtr<EventListener> listener = RBEventListener::create(listenerRB);
    selfImpl->addEventListener(type, listener.release(), useCapture);
    return Qnil;
}

VALUE RBDOMWindow::remove_event_listener(int argc, VALUE* argv, VALUE self)
{
    DOMWindow* selfImpl = impl<DOMWindow>(self);
    VALUE typeRB, listenerRB, useCaptureRB;
    rb_scan_args(argc, argv, "12", &typeRB, &listenerRB, &useCaptureRB);
    if (rb_block_given_p()) {
        useCaptureRB = listenerRB;
        listenerRB = rb_block_proc();
    }
    
    String type = StringValueCStr(typeRB);
    bool useCapture = RTEST(useCaptureRB);
    RefPtr<EventListener> listener = RBEventListener::create(listenerRB);
    selfImpl->removeEventListener(type, listener.get(), useCapture);
    return Qnil;
}

} // namespace WebCore
