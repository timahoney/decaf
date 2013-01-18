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
#include "RBHTMLFormControlsCollection.h"

#include "RBConverters.h"
#include "RBRadioNodeList.h"
#include <wtf/Vector.h>

namespace WebCore {

VALUE RBHTMLFormControlsCollection::named_item(int argc, VALUE* argv, VALUE self)
{
    VALUE name;
    rb_scan_args(argc, argv, "01", &name);

    HTMLFormControlsCollection* collection = impl<HTMLFormControlsCollection>(self);
    String propertyName = rbStringOrUndefined(name);
    Vector<RefPtr<Node> > namedItems;
    collection->namedItems(propertyName, namedItems);

    if (namedItems.isEmpty())
        return Qnil;
    if (namedItems.size() == 1)
        return toRB(namedItems[0].get());

    return toRB(collection->ownerNode()->radioNodeList(propertyName));
}


} // namespace WebCore
