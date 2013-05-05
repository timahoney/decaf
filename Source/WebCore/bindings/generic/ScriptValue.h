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

#ifndef ScriptValue_h
#define ScriptValue_h

#include "SerializedScriptValue.h"
#include "ScriptState.h"
#include "ScriptValueDelegate.h"
#include <Ruby/ruby.h>
#include <runtime/JSCJSValue.h>
#include <wtf/PassRefPtr.h>
#include <wtf/text/WTFString.h>

namespace JSC {
class JSGlobalData;
class JSValue;
}

namespace WebCore {

class InspectorValue;
class SerializedScriptValue;

class ScriptValue {
public:
    ScriptValue();
    ScriptValue(PassRefPtr<ScriptValueDelegate> delegate);

    ScriptValue(JSC::JSGlobalData&, JSC::JSValue);
    ScriptValue(VALUE);

    virtual ~ScriptValue() {}

    bool isString() const;
    bool getString(ScriptState*, String& result) const;
    String toString(ScriptState*) const;
    bool isEqual(ScriptState*, const ScriptValue&) const;
    bool isNull() const;
    bool isUndefined() const;
    bool isObject() const;
    bool isFunction() const;
    bool hasNoValue() const;
    void clear();

    // Methods for serialization.
    bool isNumber() const;
    bool isInt32() const;
    int32_t asInt32() const;
    double asDouble() const;
    double asNumber() const;
    bool isBoolean() const;
    bool isTrue() const;

    // FIXME: Rename this function to something more meaningful.
    // What is a JSCell?
    bool isCell() const;

    // FIXME: Should we remove these? They aren't very generic, but they're convenient.
    JSC::JSValue jsValue() const;
    VALUE rbValue() const;

    bool operator==(const ScriptValue& other) const;

    PassRefPtr<SerializedScriptValue> serialize(ScriptState*, SerializationErrorMode = Throwing);
    PassRefPtr<SerializedScriptValue> serialize(ScriptState*, MessagePortArray*, ArrayBufferArray*, bool&);

    ScriptValueDelegate* delegate() const { return m_delegate.get(); }
    ScriptType scriptType() const { return m_delegate->scriptType(); }

#if ENABLE(INSPECTOR)
    PassRefPtr<InspectorValue> toInspectorValue(ScriptState*) const;
#endif

protected:
    RefPtr<ScriptValueDelegate> m_delegate;
};

} // namespace WebCore

#endif // ScriptValue_h
