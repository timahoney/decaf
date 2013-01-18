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

#if ENABLE(XSLT)

#include "RBXSLTProcessor.h"

#include "RBConverters.h"

namespace WebCore {

VALUE RBXSLTProcessor::set_parameter(VALUE self, VALUE namespaceURI, VALUE localName, VALUE value)
{
    XSLTProcessor* selfImpl = impl<XSLTProcessor>(self);
    if (NIL_P(localName) || NIL_P(value))
        return Qnil;
    String namespaceURIImpl = StringValueCStr(namespaceURI);
    String localNameImpl = StringValueCStr(namespaceURI);
    String valueImpl = StringValueCStr(value);
    selfImpl->setParameter(namespaceURIImpl, localNameImpl, valueImpl);
    return Qnil;
}

VALUE RBXSLTProcessor::get_parameter(VALUE self, VALUE namespaceURI, VALUE localName)
{
    XSLTProcessor* selfImpl = impl<XSLTProcessor>(self);
    if (NIL_P(localName))
        return Qnil;
    String namespaceURIImpl = StringValueCStr(namespaceURI);
    String localNameImpl = StringValueCStr(localName);
    String value = selfImpl->getParameter(namespaceURIImpl, localNameImpl);
    return toRB(value);
}

VALUE RBXSLTProcessor::remove_parameter(VALUE self, VALUE namespaceURI, VALUE localName)
{
    XSLTProcessor* selfImpl = impl<XSLTProcessor>(self);
    if (NIL_P(localName))
        return Qnil;
    String namespaceURIImpl = StringValueCStr(namespaceURI);
    String localNameImpl = StringValueCStr(localName);
    selfImpl->removeParameter(namespaceURIImpl, localNameImpl);
    return Qnil;
}

#endif // ENABLE(XSLT)

} // namespace WebCore
