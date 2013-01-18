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
#include "RBCSSRule.h"

#include "RBConverters.h"
#include "RBCSSCharsetRule.h"
#include "RBCSSFontFaceRule.h"
#include "RBCSSImportRule.h"
#include "RBCSSMediaRule.h"
#include "RBCSSPageRule.h"
#include "RBCSSStyleRule.h"
#include "RBWebKitCSSKeyframeRule.h"
#include "RBWebKitCSSKeyframesRule.h"
#if ENABLE(CSS_REGIONS)
#include "RBWebKitCSSRegionRule.h"
#endif
#if ENABLE(CSS_DEVICE_ADAPTATION)
#include "RBWebKitCSSViewportRule.h"
#endif

namespace WebCore {

VALUE toRB(PassRefPtr<CSSRule> impl)
{
    if (!impl)
        return Qnil;

    switch (impl->type()) {
        case CSSRule::STYLE_RULE:
            return toRB(static_cast<CSSStyleRule*>(impl.get()));
        case CSSRule::MEDIA_RULE:
            return toRB(static_cast<CSSMediaRule*>(impl.get()));
        case CSSRule::FONT_FACE_RULE:
            return toRB(static_cast<CSSFontFaceRule*>(impl.get()));
        case CSSRule::PAGE_RULE:
            return toRB(static_cast<CSSPageRule*>(impl.get()));
        case CSSRule::IMPORT_RULE:
            return toRB(static_cast<CSSImportRule*>(impl.get()));
        case CSSRule::CHARSET_RULE:
            return toRB(static_cast<CSSCharsetRule*>(impl.get()));
        case CSSRule::WEBKIT_KEYFRAME_RULE:
            return toRB(static_cast<WebKitCSSKeyframeRule*>(impl.get()));
        case CSSRule::WEBKIT_KEYFRAMES_RULE:
            return toRB(static_cast<WebKitCSSKeyframesRule*>(impl.get()));
#if ENABLE(CSS_DEVICE_ADAPTATION)
        case CSSRule::WEBKIT_VIEWPORT_RULE:
            return toRB(static_cast<WebKitCSSViewportRule*>(impl.get()));
#endif
#if ENABLE(CSS_REGIONS)
        case CSSRule::WEBKIT_REGION_RULE:
            return toRB(static_cast<WebKitCSSRegionRule*>(impl.get()));
#endif
        default:
            return toRB(RBCSSRule::rubyClass(), impl);
    }
}

} // namespace WebCore
