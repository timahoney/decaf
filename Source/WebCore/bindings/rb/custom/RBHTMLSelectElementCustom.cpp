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
#include "RBHTMLSelectElement.h"

#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include "RBHTMLOptionElement.h"

namespace WebCore {

VALUE RBHTMLSelectElement::indexed_setter(VALUE self, VALUE index, VALUE value)
{
    HTMLSelectElement* selfImpl = impl<HTMLSelectElement>(self);
    HTMLOptionElement* option = impl<HTMLOptionElement>(value);
    ExceptionCode ec = 0;
    selfImpl->setOption(NUM2UINT(index), option, ec);
    RB::setDOMException(ec);
    return value;
}

VALUE RBHTMLSelectElement::remove(int argc, VALUE* argv, VALUE self)
{
    HTMLSelectElement* select = impl<HTMLSelectElement>(self);

    VALUE argument;
    rb_scan_args(argc, argv, "10", &argument);

    // The remove function can take either an option object or the index of an option.
    if (IS_RB_KIND(argument, HTMLOptionElement))
        select->remove(impl<HTMLOptionElement>(argument));
    else
        select->remove(NUM2INT(argument));

    return Qnil;
}

} // namespace WebCore
