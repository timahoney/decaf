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
#include "RBLocation.h"

#include "BindingSecurity.h"
#include "BindingState.h"
#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include "RBScriptState.h"

namespace WebCore {

// FIXME: What goes here?
RB_UNIMPLEMENTED(RBLocation::named_setter(VALUE, VALUE, VALUE))

VALUE RBLocation::to_s(VALUE self)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    Frame* frame = selfImpl->frame();
    if (!frame || !BindingSecurity::shouldAllowAccessToFrame(state, frame))
        return Qnil;

    return toRB(selfImpl->toString());
}

VALUE RBLocation::href_setter(VALUE self, VALUE newHref)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    String href = StringValueCStr(newHref);
    selfImpl->setHref(href, activeDOMWindow(state), firstDOMWindow(state));
    return newHref;
}

VALUE RBLocation::protocol_setter(VALUE self, VALUE newProtocol)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    String protocol = StringValueCStr(newProtocol);
    ExceptionCode ec = 0;
    selfImpl->setProtocol(protocol, activeDOMWindow(state), firstDOMWindow(state), ec);
    RB::setDOMException(ec);
    return newProtocol;
}

VALUE RBLocation::host_setter(VALUE self, VALUE newHost)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    String host = StringValueCStr(newHost);
    selfImpl->setHost(host, activeDOMWindow(state), firstDOMWindow(state));
    return Qnil;
}

VALUE RBLocation::hostname_setter(VALUE self, VALUE newHostname)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    String hostname = StringValueCStr(newHostname);
    selfImpl->setHostname(hostname, activeDOMWindow(state), firstDOMWindow(state));
    return Qnil;
}

VALUE RBLocation::port_setter(VALUE self, VALUE newPort)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    String port = StringValueCStr(newPort);
    selfImpl->setPort(port, activeDOMWindow(state), firstDOMWindow(state));
    return Qnil;
}

VALUE RBLocation::pathname_setter(VALUE self, VALUE newPathname)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    String pathname = StringValueCStr(newPathname);
    selfImpl->setPathname(pathname, activeDOMWindow(state), firstDOMWindow(state));
    return Qnil;
}

VALUE RBLocation::search_setter(VALUE self, VALUE newSearch)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    String pathname = StringValueCStr(newSearch);
    selfImpl->setSearch(pathname, activeDOMWindow(state), firstDOMWindow(state));
    return Qnil;
}

VALUE RBLocation::hash_setter(VALUE self, VALUE newHash)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    String hash = StringValueCStr(newHash);
    selfImpl->setHash(hash, activeDOMWindow(state), firstDOMWindow(state));
    return Qnil;
}

VALUE RBLocation::assign(int argc, VALUE* argv, VALUE self)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    VALUE url;
    rb_scan_args(argc, argv, "01", &url);

    String urlString = StringValueCStr(url);
    selfImpl->assign(urlString, activeDOMWindow(state), firstDOMWindow(state));
    return Qnil;
}

VALUE RBLocation::replace(int argc, VALUE* argv, VALUE self)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    VALUE url;
    rb_scan_args(argc, argv, "01", &url);

    String urlString = StringValueCStr(url);
    selfImpl->replace(urlString, activeDOMWindow(state), firstDOMWindow(state));
    return Qnil;
}

VALUE RBLocation::reload(VALUE self)
{
    Location* selfImpl = impl<Location>(self);
    RBScriptState* state = RBScriptState::current();
    selfImpl->reload(activeDOMWindow(state));
    return Qnil;
}

} // namespace WebCore
