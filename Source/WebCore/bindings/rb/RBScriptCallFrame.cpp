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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(JAVASCRIPT_DEBUGGER)

#include "RBScriptCallFrame.h"

#include "RBCallHelpers.h"
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

namespace WebCore {
    
RBScriptCallFrame::RBScriptCallFrame(VALUE binding, PassRefPtr<JavaScriptCallFrame> caller, intptr_t sourceID, const TextPosition& textPosition)
    : JavaScriptCallFrame(caller, sourceID, textPosition)
    , m_binding(binding)
{
    rb_gc_register_address(&m_binding);
}

RBScriptCallFrame::~RBScriptCallFrame()
{
    // FIXME: Add this back in?
    // rb_gc_unregister_address(&m_binding);
}

void RBScriptCallFrame::update(VALUE binding, intptr_t sourceID, const TextPosition& textPosition)
{
    m_textPosition = textPosition;
    m_sourceID = sourceID;
    m_binding = binding;
}

String RBScriptCallFrame::functionName() const
{
    VALUE nameSymbol = rb_funcall(m_binding, rb_intern("eval"), 1, rb_str_new2("__method__"));
    if (NIL_P(nameSymbol))
        return String();
    return rb_id2name(SYM2ID(nameSymbol));
}

String RBScriptCallFrame::type() const
{
    // FIXME: Is this correct?
    if (functionName().isEmpty())
        return "program";

    return "function";
}

VALUE RBScriptCallFrame::thisObject() const
{
    return rb_funcall(m_binding, rb_intern("eval"), 1, rb_str_new2("self"));
}

VALUE RBScriptCallFrame::evaluate(const String& script) const
{
    VALUE scriptString = rb_str_new2(script.utf8().data());
    VALUE argv[1];
    argv[0] = scriptString;
    return callFunctionProtected(m_binding, "eval", 1, argv);
}

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)
