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
#include "RBXMLHttpRequest.h"

#include "InspectorInstrumentation.h"
#include "RBArrayBuffer.h"
#include "RBArrayBufferView.h"
#include "RBBlob.h"
#include "RBConverters.h"
#include "RBDOMBinding.h"
#include "RBDOMFormData.h"
#include "RBDocument.h"
#include "RBExceptionHandler.h"
#include "RBScriptState.h"

namespace WebCore {

VALUE RBXMLHttpRequest::response_text_getter(VALUE self)
{
    ExceptionCode ec = 0;
    String text = impl<XMLHttpRequest>(self)->responseText(ec);
    if (ec) {
        rbDOMRaiseError(ec);
        return Qnil;
    }

    return text.isNull() ? Qnil : rb_str_new2(text.utf8().data());
}

VALUE RBXMLHttpRequest::response_getter(VALUE self)
{
    XMLHttpRequest* request = impl<XMLHttpRequest>(self);

    switch (request->responseTypeCode()) {
    case XMLHttpRequest::ResponseTypeDefault:
    case XMLHttpRequest::ResponseTypeText:
        return RBXMLHttpRequest::response_text_getter(self);

    case XMLHttpRequest::ResponseTypeDocument:
        {
            ExceptionCode ec = 0;
            Document* document = request->responseXML(ec);
            if (ec) {
                rbDOMRaiseError(ec);
                return Qnil;
            }
            return toRB(document);
        }

    case XMLHttpRequest::ResponseTypeBlob:
        {
            ExceptionCode ec = 0;
            Blob* blob = request->responseBlob(ec);
            if (ec) {
                rbDOMRaiseError(ec);
                return Qnil;
            }
            return toRB(blob);
        }

    case XMLHttpRequest::ResponseTypeArrayBuffer:
        {
            ExceptionCode ec = 0;
            ArrayBuffer* arrayBuffer = request->responseArrayBuffer(ec);
            if (ec) {
                rbDOMRaiseError(ec);
                return Qnil;
            }
            return toRB(arrayBuffer);
        }
    }

    return Qnil;
}

VALUE RBXMLHttpRequest::open(int argc, VALUE* argv, VALUE self)
{
    VALUE rbMethod, rbUrl, rbAsync, rbUser, rbPassword;
    rb_scan_args(argc, argv, "23", &rbMethod, &rbUrl, &rbAsync, &rbUser, &rbPassword);

    XMLHttpRequest* request = impl<XMLHttpRequest>(self);

    if (NIL_P(rbMethod) || NIL_P(rbUrl)) {
        rb_raise(rb_eArgError, "Not enough arguments to open(...)");
        return Qnil;
    }

    const KURL& url = request->scriptExecutionContext()->completeURL(StringValueCStr(rbUrl));
    String method = StringValueCStr(rbMethod);

    ExceptionCode ec = 0;
    if (NIL_P(rbAsync))
        request->open(method, url, ec);
    else if (NIL_P(rbUser))
        request->open(method, url, RTEST(rbAsync), ec);
    else if (NIL_P(rbPassword))
        request->open(method, url, RTEST(rbAsync), StringValueCStr(rbUser), ec);
    else
        request->open(method, url, RTEST(rbAsync), StringValueCStr(rbUser), StringValueCStr(rbPassword), ec);

    rbDOMRaiseError(ec);
    return Qnil;
}

VALUE RBXMLHttpRequest::send(int argc, VALUE* argv, VALUE self)
{
    XMLHttpRequest* request = impl<XMLHttpRequest>(self);
    InspectorInstrumentation::willSendXMLHttpRequest(RBDOMBinding::currentWindow()->scriptExecutionContext(), request->url());

    VALUE data;
    rb_scan_args(argc, argv, "01", &data);

    ExceptionCode ec = 0;
    if (NIL_P(data))
        request->send(ec);
    else if (IS_RB_KIND(data, Document))
        request->send(impl<Document>(data), ec);
    else if (IS_RB_KIND(data, Blob))
        request->send(impl<Blob>(data), ec);
    else if (IS_RB_KIND(data, DOMFormData))
        request->send(impl<DOMFormData>(data), ec);
    else if (IS_RB_KIND(data, ArrayBuffer))
        request->send(impl<ArrayBuffer>(data), ec);
    else if (IS_RB_KIND(data, ArrayBufferView))
        request->send(impl<ArrayBufferView>(data), ec);
    else
        request->send(StringValueCStr(data), ec);

    String sourceURL = rb_sourcefile();
    int lineNumber = rb_sourceline();
    request->setLastSendLineNumber(lineNumber);
    request->setLastSendURL(sourceURL);

    rbDOMRaiseError(ec);
    return Qnil;
}

} // namespace WebCore
