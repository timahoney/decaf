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
#include "RBDocument.h"

#include "Frame.h"
#include "RBConverters.h"
#include "RBHTMLDocument.h"
#include "RBLocation.h"
#include "RBScriptState.h"
#include "RBSVGDocument.h"

namespace WebCore {

VALUE RBDocument::location_getter(VALUE self)
{
    Document* selfImpl = impl<Document>(self);
    Frame* frame = selfImpl->frame();
    if (!frame)
        return Qnil;

    Location* location = frame->document()->domWindow()->location();
    return toRB(location);
}

VALUE RBDocument::location_setter(VALUE self, VALUE newValue)
{
    Document* selfImpl = impl<Document>(self);
    Frame* frame = selfImpl->frame();
    if (!frame)
        return Qnil;

    String locationString = rbToString(newValue);
    RBScriptState* state = RBScriptState::current();
    if (Location* location = frame->document()->domWindow()->location())
        location->setHref(locationString, activeDOMWindow(state), firstDOMWindow(state));
    return newValue;
}

VALUE toRB(Document* impl)
{
    if (!impl)
        return Qnil;

    if (impl->isHTMLDocument())
        return toRB(static_cast<HTMLDocument*>(impl));
#if ENABLE(SVG)
    else if (impl->isSVGDocument())
        return toRB(static_cast<SVGDocument*>(impl));
#endif
    else
        return toRB(RBDocument::rubyClass(), impl);
}

} // namespace WebCore
