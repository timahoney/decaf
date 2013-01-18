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
#include "RBClipboard.h"

#include "Element.h"
#include "HTMLImageElement.h"
#include "IntPoint.h"

namespace WebCore {

VALUE RBClipboard::types_getter(VALUE self)
{
    Clipboard* selfImpl = impl<Clipboard>(self);

    ListHashSet<String> types = selfImpl->types();
    if (types.isEmpty())
        return Qnil;

    VALUE array = rb_ary_new2(types.size());
    ListHashSet<String>::const_iterator end = types.end();
    for (ListHashSet<String>::const_iterator it = types.begin(); it != end; ++it)
        rb_ary_push(array, rb_str_new2((*it).utf8().data()));
    return array;
}

VALUE RBClipboard::clear_data(int argc, VALUE* argv, VALUE self)
{
    VALUE type;
    rb_scan_args(argc, argv, "01", &type);
    
    Clipboard* selfImpl = impl<Clipboard>(self);
    if (NIL_P(type))
        selfImpl->clearAllData();
    else
        selfImpl->clearData(StringValueCStr(type));
    
    return Qnil;
}

VALUE RBClipboard::set_drag_image(VALUE self, VALUE image, VALUE rbX, VALUE rbY)
{
    Clipboard* selfImpl = impl<Clipboard>(self);

    if (!selfImpl->isForDragAndDrop())
        return Qnil;

    int x = NUM2INT(rbX);
    int y = NUM2INT(rbY);

    Node* node = impl<Node>(image);
    if (!node->isElementNode()) {
        rb_raise(rb_eTypeError, "setDragImageFromElement: Invalid first argument");
        return Qnil;
    }

    if (static_cast<Element*>(node)->hasLocalName(HTMLNames::imgTag) && !node->inDocument())
        selfImpl->setDragImage(static_cast<HTMLImageElement*>(node)->cachedImage(), IntPoint(x, y));
    else
        selfImpl->setDragImageElement(node, IntPoint(x, y));

    return Qnil;
}

} // namespace WebCore
