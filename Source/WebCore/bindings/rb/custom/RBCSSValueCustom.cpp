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
#include "RBCSSValue.h"

#include "RBConverters.h"
#include "RBCSSPrimitiveValue.h"
#include "RBCSSValueList.h"
#include "RBNode.h"
#include "RBWebKitCSSTransformValue.h"
#if ENABLE(CSS_FILTERS)
#include "RBWebKitCSSFilterValue.h"
#endif
#if ENABLE(SVG)
#include "RBSVGColor.h"
#include "RBSVGPaint.h"
#endif
#include <wtf/Assertions.h>

namespace WebCore {

VALUE toRB(PassRefPtr<CSSValue> impl)
{
    if (!impl)
        return Qnil;

    // Scripts should only ever see cloned CSSValues, never the internal ones.
    ASSERT(impl->isCSSOMSafe());

    // If we're here under erroneous circumstances, prefer returning null over a potentially insecure value.
    if (!impl->isCSSOMSafe())
        return Qnil;

    if (impl->isWebKitCSSTransformValue())
        return toRB(static_cast<WebKitCSSTransformValue*>(impl.get()));
#if ENABLE(CSS_FILTERS)
    else if (impl->isWebKitCSSFilterValue())
        return toRB(static_cast<WebKitCSSFilterValue*>(impl.get()));
#endif
    else if (impl->isValueList())
        return toRB(static_cast<CSSValueList*>(impl.get()));
#if ENABLE(SVG)
    else if (impl->isSVGPaint())
        return toRB(static_cast<SVGPaint*>(impl.get()));
    else if (impl->isSVGColor())
        return toRB(static_cast<SVGColor*>(impl.get()));
#endif
    else if (impl->isPrimitiveValue())
        return toRB(static_cast<CSSPrimitiveValue*>(impl.get()));
    else
        return toRB(RBCSSValue::rubyClass(), impl);
}

} // namespace WebCore
