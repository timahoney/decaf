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

#ifndef ScriptValueDelegate_h
#define ScriptValueDelegate_h

#include "InspectorValues.h"
#include "SerializedScriptValue.h"
#include "ScriptState.h"
#include "ScriptType.h"
#include <wtf/PassRefPtr.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class ScriptValue;
class SerializedScriptValue;

class ScriptValueDelegate : public RefCounted<ScriptValueDelegate> {
public:
    virtual ~ScriptValueDelegate() {}

    virtual bool isString() const = 0;
    virtual bool getString(ScriptState*, String& result) const = 0;
    virtual String toString(ScriptState*) const = 0;
    virtual bool isEqual(ScriptState*, const ScriptValue&) const = 0;
    virtual bool isNull() const = 0;
    virtual bool isUndefined() const = 0;
    virtual bool isObject() const = 0;
    virtual bool isFunction() const = 0;
    virtual bool hasNoValue() const = 0;
    virtual void clear() = 0;

    virtual bool isNumber() const = 0;
    virtual bool isInt32() const = 0;
    virtual int32_t asInt32() const = 0;
    virtual double asDouble() const = 0;
    virtual bool isBoolean() const = 0;
    virtual bool isTrue() const = 0;
    virtual bool isCell() const = 0;
    double asNumber() const { return isInt32() ? asInt32() : asDouble(); }

    virtual PassRefPtr<SerializedScriptValue> serialize(ScriptState*, SerializationErrorMode = Throwing) = 0;
    virtual PassRefPtr<SerializedScriptValue> serialize(ScriptState*, MessagePortArray*, ArrayBufferArray*, bool&) = 0;

#if ENABLE(INSPECTOR)
    virtual PassRefPtr<InspectorValue> toInspectorValue(ScriptState*) const = 0;
#endif

    ScriptType scriptType() const { return m_type; }

    virtual bool operator==(const ScriptValueDelegate& other) const = 0;

protected:
    ScriptValueDelegate(ScriptType type) : m_type(type) { }

private:
    ScriptType m_type;
};

class EmptyScriptValueDelegate : public ScriptValueDelegate {
public:
    static PassRefPtr<EmptyScriptValueDelegate> create()
    {
        return adoptRef(new EmptyScriptValueDelegate());
    }

    virtual ~EmptyScriptValueDelegate() {}

    virtual bool isString() const { return false; }
    virtual bool getString(ScriptState*, String&) const { return false; }
    virtual String toString(ScriptState*) const { return String(); }
    virtual bool isEqual(ScriptState*, const ScriptValue&) const;
    virtual bool isNull() const { return true; }
    virtual bool isUndefined() const { return true; }
    virtual bool isObject() const { return false; }
    virtual bool isFunction() const { return false; }
    virtual bool hasNoValue() const { return true; }
    virtual bool isNumber() const { return false; }
    virtual bool isInt32() const { return false; }
    virtual int32_t asInt32() const { return 0; }
    virtual double asDouble() const { return 0; }
    virtual bool isBoolean() const { return false; }
    virtual bool isTrue() const { return false; }
    virtual bool isCell() const { return false; }

    virtual void clear() { }

    virtual bool operator==(const ScriptValueDelegate& other) const;

    virtual PassRefPtr<SerializedScriptValue> serialize(ScriptState*, SerializationErrorMode = Throwing) { return SerializedScriptValue::create(); }
    virtual PassRefPtr<SerializedScriptValue> serialize(ScriptState*, MessagePortArray*, ArrayBufferArray*, bool&) { return SerializedScriptValue::create(); }

#if ENABLE(INSPECTOR)
    virtual PassRefPtr<InspectorValue> toInspectorValue(ScriptState*) const { return InspectorValue::null(); }
#endif

private:
    EmptyScriptValueDelegate() : ScriptValueDelegate(RBScriptType) { }
};

} // namespace WebCore

#endif // ScriptValueDelegate_h
