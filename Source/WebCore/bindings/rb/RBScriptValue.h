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

#ifndef RBScriptValue_h
#define RBScriptValue_h

#include "RBConverters.h"
#include "ScriptValueDelegate.h"
#include "ScriptValue.h"
#include <Ruby/ruby.h>

namespace WebCore {

class RBScriptValue : public ScriptValueDelegate {
public:
    static PassRefPtr<RBScriptValue> create(VALUE object)
    {
        return adoptRef(new RBScriptValue(object));
    }
    
    static ScriptValue scriptValue(VALUE object)
    {
        return ScriptValue(RBScriptValue::create(object));
    }

    virtual ~RBScriptValue();

    VALUE rbValue() const { return m_value; }

    virtual bool isString() const;
    virtual bool getString(ScriptState*, String& result) const;
    virtual String toString(ScriptState*) const;
    virtual bool isEqual(ScriptState*, const ScriptValue& other) const;
    virtual bool isNull() const { return NIL_P(m_value); }
    virtual bool isUndefined() const { return NIL_P(m_value); }
    virtual bool isObject() const;
    virtual bool isFunction() const { return RTEST(rb_obj_is_kind_of(m_value, rb_cProc)); }
    virtual bool hasNoValue() const { return NIL_P(m_value); }

    virtual bool isNumber() const { return IS_RB_NUM(m_value); };
    virtual bool isInt32() const { return IS_RB_INT(m_value); };
    virtual int32_t asInt32() const { return NUM2DBL(m_value); };
    virtual double asDouble() const { return NUM2DBL(m_value); };
    virtual bool isBoolean() const { return TYPE(m_value) == T_TRUE || TYPE(m_value) == T_FALSE; };
    virtual bool isTrue() const { return RTEST(m_value); };
    virtual bool isCell() const;

    virtual void clear() { m_value = Qnil; }

    virtual bool operator==(const ScriptValueDelegate& other) const;
    
    virtual PassRefPtr<SerializedScriptValue> serialize(ScriptState*, SerializationErrorMode = Throwing);
    virtual PassRefPtr<SerializedScriptValue> serialize(ScriptState*, MessagePortArray*, ArrayBufferArray*, bool&);

#if ENABLE(INSPECTOR)
    virtual PassRefPtr<InspectorValue> toInspectorValue(ScriptState*) const;
#endif

private:
    RBScriptValue(VALUE object);

    VALUE m_value;    
};

inline VALUE toRB(const ScriptValue& value)
{
    return value.rbValue();
}

} // namespace WebCore

#endif // RBScriptValue_h
