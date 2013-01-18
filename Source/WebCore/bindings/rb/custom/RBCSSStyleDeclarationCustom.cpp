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
#include "RBCSSStyleDeclaration.h"

#include "RBExceptionHandler.h"

namespace WebCore {

VALUE RBCSSStyleDeclaration::named_setter(VALUE self, VALUE name, VALUE value)
{
    CSSStyleDeclaration* selfImpl = impl<CSSStyleDeclaration>(self);
    ExceptionCode ec = 0;
    String nameImpl = StringValueCStr(name);
    String valueImpl = StringValueCStr(value);
    selfImpl->setProperty(nameImpl, valueImpl, "", ec);
    rbDOMRaiseError(ec);
    return value;
}

} // namespace WebCore
