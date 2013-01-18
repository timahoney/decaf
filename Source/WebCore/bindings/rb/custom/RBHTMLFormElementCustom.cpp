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
#include "RBHTMLFormElement.h"

#include "RBNodeList.h"
#include "StaticNodeList.h"

namespace WebCore {

VALUE RBHTMLFormElement::named_getter(VALUE self, VALUE name)
{
    HTMLFormElement* form = impl<HTMLFormElement>(self);
    
    Vector<RefPtr<Node>> namedItems;
    String propertyName = String(StringValueCStr(name));
    form->getNamedElements(propertyName, namedItems);
    
    if (namedItems.isEmpty())
        return Qnil;
    if (namedItems.size() == 1)
        return toRB(namedItems[0].get());

    // FIXME: HTML5 specifies that this should be a RadioNodeList.
    return toRB(StaticNodeList::adopt(namedItems).get());
}

} // namespace WebCore
