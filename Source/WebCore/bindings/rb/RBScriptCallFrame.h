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

#ifndef RBScriptCallFrame_h
#define RBScriptCallFrame_h

#if ENABLE(JAVASCRIPT_DEBUGGER)

#include "JavaScriptCallFrame.h"
#include <Ruby/ruby.h>

namespace WebCore {

class RBScriptCallFrame : public JavaScriptCallFrame {
public:
    static PassRefPtr<RBScriptCallFrame> create(VALUE binding, PassRefPtr<JavaScriptCallFrame> caller, intptr_t sourceID, const TextPosition& textPosition)
    {
        return adoptRef(new RBScriptCallFrame(binding, caller, sourceID, textPosition));
    }

    void update(VALUE binding, intptr_t sourceID, const TextPosition& textPosition);

    virtual String functionName() const;
    
    String type() const;
    VALUE thisObject() const;
    VALUE evaluate(const String& script) const;
    
    VALUE binding() const { return m_binding; }
    
private:
    RBScriptCallFrame(VALUE binding, PassRefPtr<JavaScriptCallFrame> caller, intptr_t sourceID, const TextPosition&);
    virtual ~RBScriptCallFrame();

    VALUE m_binding;
};

} // namespace WebCore

#endif // ENABLE(JAVASCRIPT_DEBUGGER)

#endif // JSScriptCallFrame_h
