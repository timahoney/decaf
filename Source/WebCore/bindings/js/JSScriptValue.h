/*
 * Copyright (c) 2008, 2011 Google Inc. All rights reserved.
 * Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JSScriptValue_h
#define JSScriptValue_h

#include "ScriptValueDelegate.h"
#include "ScriptValue.h"
#include <heap/Strong.h>
#include <heap/StrongInlines.h>
#include <runtime/JSCJSValue.h>

namespace WebCore {

class JSScriptValue : public ScriptValueDelegate {
public:
    static PassRefPtr<JSScriptValue> create(JSC::JSGlobalData& globalData, JSC::JSValue value)
    {
        return adoptRef(new JSScriptValue(globalData, value));
    }

    static ScriptValue scriptValue(JSC::JSGlobalData& globalData, JSC::JSValue value)
    {
        return ScriptValue(JSScriptValue::create(globalData, value));
    }

    virtual ~JSScriptValue();

    JSC::JSValue jsValue() const { return m_value.get(); }

    virtual bool isString() const;
    virtual bool getString(ScriptState*, String& result) const;
    virtual String toString(ScriptState*) const;
    virtual bool isEqual(ScriptState*, const ScriptValue&) const;
    virtual bool isNull() const;
    virtual bool isUndefined() const;
    virtual bool isObject() const;
    virtual bool isFunction() const;
    virtual bool hasNoValue() const { return !m_value; }
    virtual bool isNumber() const;
    virtual bool isInt64() const;
    virtual int64_t asInt64() const;
    virtual double asDouble() const;
    virtual bool isBoolean() const;
    virtual bool isTrue() const;
    virtual bool isCell() const;

    virtual void clear() { m_value.clear(); }

    virtual bool operator==(const ScriptValueDelegate& other) const;

    virtual PassRefPtr<SerializedScriptValue> serialize(ScriptState*, SerializationErrorMode = Throwing);
    virtual PassRefPtr<SerializedScriptValue> serialize(ScriptState*, MessagePortArray*, ArrayBufferArray*, bool&);

#if ENABLE(INSPECTOR)
    virtual PassRefPtr<InspectorValue> toInspectorValue(ScriptState*) const;
#endif

private:
    JSScriptValue(JSC::JSGlobalData& globalData, JSC::JSValue value)
    : ScriptValueDelegate(JSScriptType)
    , m_value(globalData, value)
    {
    }

    JSC::Strong<JSC::Unknown> m_value;
};

} // namespace WebCore

#endif // JSScriptValue_h
