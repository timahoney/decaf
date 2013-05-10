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
#include "RBElement.h"

#include "Element.h"
#include "HTMLElement.h"
#include "RBHTMLElementWrapperFactory.h"
#if ENABLE(SVG)
#include "RBSVGElement.h"
#include "RBSVGElementWrapperFactory.h"
#endif
#include <wtf/PassRefPtr.h>

namespace WebCore {

inline VALUE createRBElementWrapper(Element* element)
{
    if (!element)
        return Qnil;
    
    if (element->isHTMLElement())
        return createRBHTMLWrapper(toHTMLElement(element));
    
#if ENABLE(SVG)
    if (element->isSVGElement())
        return createRBSVGWrapper(static_cast<SVGElement*>(element));
#endif
    
    return toRB(RBElement::rubyClass(), element);
}

VALUE toRB(Element* impl)
{
    return createRBElementWrapper(impl);
}

} // namespace WebCore
