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
#include "RBHTMLAllCollection.h"

#include "RBConverters.h"
#include "StaticNodeList.h"
#include <wtf/Vector.h>

namespace WebCore {

VALUE RBHTMLAllCollection::item(int argc, VALUE* argv, VALUE self)
{
    VALUE index;
    rb_scan_args(argc, argv, "01", &index);

    return toRB(impl<HTMLAllCollection>(self)->item(NIL_P(index) ? 0 : NUM2UINT(index)));
}

VALUE RBHTMLAllCollection::named_item(VALUE self, VALUE name)
{
    Vector<RefPtr<Node> > namedItems;
    impl<HTMLAllCollection>(self)->namedItems(StringValueCStr(name), namedItems);

    if (namedItems.isEmpty())
        return Qnil;
    if (namedItems.size() == 1)
        return toRB(namedItems[0].get());

    // FIXME: HTML5 specification says this should be a HTMLCollection.
    // http://www.whatwg.org/specs/web-apps/current-work/multipage/common-dom-interfaces.html#htmlallcollection
    return toRB(StaticNodeList::adopt(namedItems).get());
}

} // namespace WebCore
