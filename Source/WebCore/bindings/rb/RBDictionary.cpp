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
#include "RBDictionary.h"

#include "ArrayValue.h"
#include "Dictionary.h"
#include "DOMWindow.h"
#include "EventTarget.h"
#include "MediaKeyError.h"
#include "Node.h"
#include "RBConverters.h"
#include "RBObject.h"
#include "RBScriptValue.h"
#include "SerializedScriptValue.h"
#include "Storage.h"
#include "TrackBase.h"
#include <wtf/HashMap.h>
#include <wtf/MathExtras.h>
#include <wtf/text/AtomicString.h>
#include <wtf/Uint8Array.h>

namespace WebCore {

RBDictionary::RBDictionary(VALUE rubyHash)
{
    if (TYPE(rubyHash) == T_HASH) {
        m_hash = rubyHash;
        rb_gc_register_address(&m_hash);
    } else {
        m_hash = Qnil;
    }
}

RBDictionary::~RBDictionary()
{
    // FIXME: Should we put this back in?
    // rb_gc_unregister_address(&m_hash);
}

bool RBDictionary::getWithUndefinedOrNullCheck(const String& propertyName, String& value) const
{
    VALUE hashValue = rb_hash_aref(m_hash, ID2SYM(rb_intern(propertyName.utf8().data())));
    if (NIL_P(hashValue))
        return false;

    value = rbToString(hashValue);
    return true;
}

bool RBDictionary::getOwnPropertiesAsStringHashMap(HashMap<String, String>& map) const
{
    // FIXME: Do we need to convert from underscore_case to camelCase?
    VALUE keys = rb_funcall(m_hash, rb_intern("keys"), 0);
    size_t count = RARRAY_LEN(keys);
    for (size_t i = 0; i < count; i++) {
        VALUE keyRB = rb_ary_entry(keys, i);
        VALUE valueRB = rb_hash_aref(m_hash, keyRB);
        if (!SYMBOL_P(keyRB))
            continue;
        String key = rbToString(keyRB);
        String value = rbToString(valueRB);
        map.set(key, value);
    }
    return true;
}

bool RBDictionary::getOwnPropertyNames(Vector<String>& list) const
{
    // FIXME: Do we need to convert from underscore_case to camelCase?
    VALUE keys = rb_funcall(m_hash, rb_intern("keys"), 0);
    size_t count = RARRAY_LEN(keys);
    for (size_t i = 0; i < count; i++) {
        VALUE keyRB = rb_ary_entry(keys, i);
        if (!SYMBOL_P(keyRB))
            continue;
        String key = rbToString(keyRB);
        list.append(key);
    }
    return true;
}

void RBDictionary::convertValue(VALUE value, bool& result)
{
    result = RTEST(value);
}

void RBDictionary::convertValue(VALUE value, int& result)
{
    result = NUM2INT(value);
}

void RBDictionary::convertValue(VALUE value, unsigned& result)
{
    result = NUM2UINT(value);
}

void RBDictionary::convertValue(VALUE value, unsigned short& result)
{
    result = NUM2UINT(value);
}

void RBDictionary::convertValue(VALUE value, unsigned long long& result)
{
    result = NUM2ULL(value);
}

void RBDictionary::convertValue(VALUE value, double& result)
{
    result = NUM2DBL(value);
}

void RBDictionary::convertValue(VALUE value, Dictionary& result)
{
    result = Dictionary(value);
}

void RBDictionary::convertValue(VALUE value, String& result)
{
    result = StringValueCStr(value);
}

void RBDictionary::convertValue(VALUE value, ScriptValue& result)
{
    result = RBScriptValue::scriptValue(value);
}

void RBDictionary::convertValue(VALUE value, Vector<String>& result)
{
    result = rbToNativeArray<String>(value);
}

void RBDictionary::convertValue(VALUE value, RefPtr<SerializedScriptValue>& result)
{
    result = SerializedScriptValue::create(value);
}

void RBDictionary::convertValue(VALUE value, RefPtr<DOMWindow>& result)
{
    result = impl<DOMWindow>(value);
}

void RBDictionary::convertValue(VALUE value, RefPtr<EventTarget>& result)
{
    result = impl<EventTarget>(value);
}

void RBDictionary::convertValue(VALUE value, RefPtr<Node>& result)
{
    result = impl<Node>(value);
}

void RBDictionary::convertValue(VALUE value, RefPtr<Storage>& result)
{
    result = impl<Storage>(value);
}

void RBDictionary::convertValue(VALUE value, MessagePortArray& result)
{
    size_t size = RARRAY_LEN(value);
    for (size_t i = 0; i < size; i++) {
        VALUE portRB = rb_ary_entry(value, i);
        RefPtr<MessagePort> port = impl<MessagePort>(portRB);
        result.append(port);
    }
}

#if ENABLE(VIDEO_TRACK)
void RBDictionary::convertValue(VALUE value, RefPtr<TrackBase>& result)
{
    result = impl<TrackBase>(value);
}
#endif

void RBDictionary::convertValue(VALUE value, HashSet<AtomicString>& result)
{
    size_t size = RARRAY_LEN(value);
    for (size_t i = 0; i < size; i++) {
        VALUE stringRB = rb_ary_entry(value, i);
        String string = rbToString(stringRB);
        result.add(string);
    }
}

void RBDictionary::convertValue(VALUE value, ArrayValue& result)
{
    result =  ArrayValue(value);
}

void RBDictionary::convertValue(VALUE value, RefPtr<Uint8Array>& result)
{
    result = impl<Uint8Array>(value);
}

#if ENABLE(ENCRYPTED_MEDIA)
void RBDictionary::convertValue(VALUE value, RefPtr<MediaKeyError>& result)
{
    result = impl<MediaKeyError>(value);
}
#endif

} // namespace WebCore
