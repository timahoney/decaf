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
#include "RBHTMLFrameSetElement.h"

#include "RBConverters.h"
#include "HTMLCollection.h"
#include "HTMLFrameElement.h"

namespace WebCore {

VALUE RBHTMLFrameSetElement::named_getter(VALUE self, VALUE name)
{
    HTMLElement* element = impl<HTMLElement>(self);
    String propertyName = String(StringValueCStr(name));
    Node* frameElement = element->children()->namedItem(propertyName);
    if (Document* document = static_cast<HTMLFrameElement*>(frameElement)->contentDocument()) {
        if (DOMWindow* window = document->domWindow())
            return toRB(window);
    }
    
    return Qnil;
}

} // namespace WebCore
