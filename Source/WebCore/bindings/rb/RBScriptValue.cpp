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
#include "RBScriptValue.h"

#include "RBConverters.h"
#include "RBDOMBinding.h"
#include "RBObject.h"
#include <Ruby/intern.h>

namespace WebCore {

RBScriptValue::RBScriptValue(VALUE object)
    : ScriptValueDelegate(RBScriptType)
    , m_value(object)
{
    rb_gc_register_address(&m_value);
}

RBScriptValue::~RBScriptValue()
{
    // FIXME: Add this back in?
    // rb_gc_unregister_address(&m_value);
}

bool RBScriptValue::getString(ScriptState* scriptState, String& result) const
{
    result = toString(scriptState);
    return true;
}

String RBScriptValue::toString(ScriptState*) const
{
    return rbToString(m_value);
}

bool RBScriptValue::isEqual(ScriptState*, const ScriptValue& other) const
{
    return RTEST(rb_equal(m_value, static_cast<RBScriptValue*>(other.delegate())->m_value));
}

bool RBScriptValue::isObject() const
{
    switch (TYPE(m_value)) {
    case T_FIXNUM:
    case T_BIGNUM:
    case T_TRUE:
    case T_FALSE:
    case T_STRING:
    case T_FLOAT:
    case T_NIL:
        return false;

    default:
        return true;
    }
}

PassRefPtr<SerializedScriptValue> RBScriptValue::serialize(ScriptState*, SerializationErrorMode)
{
    return SerializedScriptValue::create(m_value);
}

PassRefPtr<SerializedScriptValue> RBScriptValue::serialize(ScriptState*, MessagePortArray* messagePorts, ArrayBufferArray* arrayBuffers, bool& didThrow)
{
    RefPtr<SerializedScriptValue> serializedScriptValue = SerializedScriptValue::create(m_value, messagePorts, arrayBuffers);
    didThrow = !NIL_P(rb_errinfo());
    return serializedScriptValue.release();
}

ScriptValue RBScriptValue::deserialize(ScriptState*, SerializedScriptValue* serializedScriptValue, SerializationErrorMode)
{
    VALUE rbValue = serializedScriptValue->deserializeRB();
    return RBScriptValue::scriptValue(rbValue);
}

#if ENABLE(INSPECTOR)
static PassRefPtr<InspectorValue> rbToInspectorValue(VALUE value, int maxDepth)
{
    if (!maxDepth)
        return 0;
    maxDepth--;

    if (NIL_P(value))
        return InspectorValue::null();
    if (IS_RB_BOOL(value))
        return InspectorBasicValue::create(RTEST(value));
    if (IS_RB_FLOAT(value))
        return InspectorBasicValue::create(NUM2DBL(value));
    if (IS_RB_STRING(value))
        return InspectorString::create(StringValueCStr(value));
    if (IS_RB_INT(value))
        return InspectorBasicValue::create(static_cast<double>(NUM2LL(value)));
    
    if (IS_RB_SYMBOL(value)) {
        String name = rbToString(value);
        return InspectorString::create(name);
    }
    
    if (IS_RB_HASH(value)) {
        RefPtr<InspectorObject> inspectorObject = InspectorObject::create();
        VALUE keys = rb_funcall(value, rb_intern("keys"), 0);
        size_t count = RARRAY_LEN(keys);
        for (size_t i = 0; i < count; i++) {
            VALUE key = rb_ary_entry(keys, i);
            VALUE hashValue = rb_hash_aref(value, key);
            RefPtr<InspectorValue> inspectorValue = rbToInspectorValue(hashValue, maxDepth);
            if (!inspectorValue)
                return 0;
            String name = rbToString(key);
            inspectorObject->setValue(name, inspectorValue);
        }
        return inspectorObject;
    }

    if (IS_RB_ARRAY(value)) {
        RefPtr<InspectorArray> inspectorArray = InspectorArray::create();
        unsigned length = RARRAY_LEN(value);
        for (unsigned i = 0; i < length; i++) {
            VALUE element = rb_ary_entry(value, i);
            RefPtr<InspectorValue> elementValue = rbToInspectorValue(element, maxDepth);
            if (!elementValue)
                return 0;
            inspectorArray->pushValue(elementValue);
        }
        return inspectorArray;
    }

    RefPtr<InspectorObject> inspectorObject = InspectorObject::create();
    VALUE ivars = rb_obj_instance_variables(value);
    size_t count = RARRAY_LEN(ivars);
    for (size_t i = 0; i < count; i++) {
        VALUE name = rb_ary_entry(ivars, i);
        if (!SYMBOL_P(name))
            name = rb_funcall(name, rb_intern("to_sym"), 0);
        ID nameId = SYM2ID(name);
        VALUE ivarValue = rb_ivar_get(value, nameId);
        RefPtr<InspectorValue> inspectorValue = rbToInspectorValue(ivarValue, maxDepth);
        if (!inspectorValue)
            return 0;
        
        String nameString = rb_id2name(nameId);
        nameString.remove(0);
        inspectorObject->setValue(nameString, inspectorValue);
    }
    return inspectorObject;
}

PassRefPtr<InspectorValue> RBScriptValue::toInspectorValue(ScriptState*) const
{
    return rbToInspectorValue(m_value, InspectorValue::maxDepth);
}
#endif
    
} // namespace WebCore
