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
#include "RBHTMLCanvasElement.h"

#include "InspectorInstrumentation.h"
#include "RBCanvasRenderingContext.h"
#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include "RBScriptState.h"
#include "RBScriptValue.h"
#include "ScriptObject.h"
#include "WebGLContextAttributes.h"

namespace WebCore {

VALUE RBHTMLCanvasElement::to_data_url(int argc, VALUE* argv, VALUE self)
{
    VALUE rbType;
    rb_scan_args(argc, argv, "01", &rbType);

    HTMLCanvasElement* selfImpl = impl<HTMLCanvasElement>(self);
    const String& type = rbStringOrNullString(rbType);
    double quality = 0;
    ExceptionCode ec = 0;
    String result = selfImpl->toDataURL(type, &quality, ec);
    rbDOMRaiseError(ec);
    return toRB(result);
}

VALUE RBHTMLCanvasElement::get_context(int argc, VALUE* argv, VALUE self)
{
    VALUE rbContextId, rbWebGLAttributes;
    rb_scan_args(argc, argv, "02", &rbContextId, &rbWebGLAttributes);

    const String& contextId = rbStringOrUndefined(rbContextId);
    HTMLCanvasElement* canvas = impl<HTMLCanvasElement>(self);
    RBScriptState* state = RBScriptState::current();
    RefPtr<CanvasContextAttributes> attrs;
#if ENABLE(WEBGL)
    if (contextId == "experimental-webgl" || contextId == "webkit-3d") {
        attrs = WebGLContextAttributes::create();
        WebGLContextAttributes* webGLAttrs = static_cast<WebGLContextAttributes*>(attrs.get());
        if (!NIL_P(rbWebGLAttributes) && IS_RB_HASH(rbWebGLAttributes)) {
            VALUE alpha = rb_hash_aref(rbWebGLAttributes, ID2SYM(rb_intern("alpha")));
            if (!NIL_P(alpha))
                webGLAttrs->setAlpha(RTEST(alpha));
            VALUE depth = rb_hash_aref(rbWebGLAttributes, ID2SYM(rb_intern("depth")));
            if (!NIL_P(depth))
                webGLAttrs->setDepth(RTEST(depth));
            VALUE stencil = rb_hash_aref(rbWebGLAttributes, ID2SYM(rb_intern("stencil")));
            if (!NIL_P(stencil))
                webGLAttrs->setStencil(RTEST(stencil));
            VALUE antialias = rb_hash_aref(rbWebGLAttributes, ID2SYM(rb_intern("antialias")));
            if (!NIL_P(antialias))
                webGLAttrs->setAntialias(RTEST(antialias));
            VALUE premultipliedAlpha = rb_hash_aref(rbWebGLAttributes, ID2SYM(rb_intern("premultipliedAlpha")));
            if (!NIL_P(premultipliedAlpha))
                webGLAttrs->setPremultipliedAlpha(RTEST(premultipliedAlpha));
            VALUE preserveDrawingBuffer = rb_hash_aref(rbWebGLAttributes, ID2SYM(rb_intern("preserveDrawingBuffer")));
            if (!NIL_P(preserveDrawingBuffer))
                webGLAttrs->setPreserveDrawingBuffer(RTEST(preserveDrawingBuffer));
        }
    }
#endif
    CanvasRenderingContext* context = canvas->getContext(contextId, attrs.get());
    if (!context)
        return Qnil;
    VALUE rbContext = toRB(context);
    if (InspectorInstrumentation::canvasAgentEnabled(canvas->document())) {
        ScriptObject contextObject(state, RBScriptValue::scriptValue(rbContext));
        ScriptObject wrapped;
        if (context->is2d())
            wrapped = InspectorInstrumentation::wrapCanvas2DRenderingContextForInstrumentation(canvas->document(), contextObject);
#if ENABLE(WEBGL)
        else if (context->is3d())
            wrapped = InspectorInstrumentation::wrapWebGLRenderingContextForInstrumentation(canvas->document(), contextObject);
#endif
        if (!wrapped.hasNoValue())
            return static_cast<RBScriptValue*>(wrapped.delegate())->rbValue();
    }

    return rbContext;
}


} // namespace WebCore
