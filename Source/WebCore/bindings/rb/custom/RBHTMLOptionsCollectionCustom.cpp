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
#include "RBHTMLOptionsCollection.h"

#include "RBConverters.h"
#include "HTMLOptionElement.h"
#include "HTMLSelectElement.h"
#include "RBExceptionHandler.h"
#include "StaticNodeList.h"
#include <wtf/Vector.h>

namespace WebCore {

VALUE RBHTMLOptionsCollection::indexed_setter(VALUE self, VALUE index, VALUE value)
{
    ExceptionCode ec = 0;
    HTMLOptionsCollection *collection = impl<HTMLOptionsCollection>(self);
    HTMLSelectElement *select = toHTMLSelectElement(collection->ownerNode());
    select->setOption(NUM2UINT(index), impl<HTMLOptionElement>(value), ec);
    RB::setDOMException(ec);
    return value;
}

VALUE RBHTMLOptionsCollection::length_setter(VALUE self, VALUE rbNewLength)
{
    ExceptionCode ec = 0;
    unsigned newLength = 0;
    double lengthValue = NUM2DBL(rbNewLength);
    if (lengthValue < 0.0)
        ec = INDEX_SIZE_ERR;
    else if (lengthValue > static_cast<double>(UINT_MAX))
        newLength = UINT_MAX;
    else
        newLength = static_cast<unsigned>(lengthValue);
    if (!ec)
        impl<HTMLOptionsCollection>(self)->setLength(newLength, ec);
    RB::setDOMException(ec);
    return rbNewLength;
}

VALUE RBHTMLOptionsCollection::named_item(int argc, VALUE* argv, VALUE self)
{
    VALUE propertyName;
    rb_scan_args(argc, argv, "01", &propertyName);
    HTMLOptionsCollection* collection = impl<HTMLOptionsCollection>(self);
    Vector<RefPtr<Node> > namedItems;
    const AtomicString& name = rbStringOrUndefined(propertyName);
    collection->namedItems(name, namedItems);

    if (namedItems.isEmpty())
        return Qnil;
    if (namedItems.size() == 1)
        return toRB(namedItems[0].get());

    // FIXME: HTML5 specifies that this should be a LiveNodeList.
    return toRB(StaticNodeList::adopt(namedItems).get());
}

VALUE RBHTMLOptionsCollection::add(int argc, VALUE* argv, VALUE self)
{
    VALUE rbOption, rbIndex;
    rb_scan_args(argc, argv, "11", &rbOption, &rbIndex);
    HTMLOptionsCollection* collection = impl<HTMLOptionsCollection>(self);
    HTMLOptionElement* option = impl<HTMLOptionElement>(rbOption);
    ExceptionCode ec = 0;

    if (NIL_P(rbIndex))
        collection->add(option, ec);
    else
        collection->add(option, NUM2UINT(rbIndex), ec);
    RB::setDOMException(ec);
    return Qnil;
}

VALUE RBHTMLOptionsCollection::remove(int argc, VALUE* argv, VALUE self)
{
    VALUE index;
    rb_scan_args(argc, argv, "01", &index);
    impl<HTMLOptionsCollection>(self)->remove(NIL_P(index) ? 0 : NUM2UINT(index));
    return Qnil;
}

} // namespace WebCore
