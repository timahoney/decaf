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

#ifndef RBDictionary_h
#define RBDictionary_h

#include "MessagePort.h"
#include <Ruby/ruby.h>
#include <wtf/Forward.h>

namespace WebCore {

class ArrayValue;
class Dictionary;
class DOMWindow;
class EventTarget;
class MediaKeyError;
class Node;
class ScriptValue;
class SerializedScriptValue;
class Storage;
class TrackBase;

class RBDictionary {
public:
    RBDictionary(VALUE rubyHash);
    ~RBDictionary();

    // Returns true if the property was found in the dictionary, and the value could be converted to the desired type.
    template <typename Result>
    bool get(const char* propertyName, Result&) const;
    bool getWithUndefinedOrNullCheck(const String& propertyName, String& value) const;

    bool getOwnPropertiesAsStringHashMap(HashMap<String, String>&) const;
    bool getOwnPropertyNames(Vector<String>&) const;

    bool isValid() const { return !NIL_P(m_hash); }

    static void convertValue(VALUE value, bool& result);
    static void convertValue(VALUE value, int& result);
    static void convertValue(VALUE value, unsigned& result);
    static void convertValue(VALUE value, unsigned short& result);
    static void convertValue(VALUE value, unsigned long long& result);
    static void convertValue(VALUE value, double& result);
    static void convertValue(VALUE value, Dictionary& result);
    static void convertValue(VALUE value, String& result);
    static void convertValue(VALUE value, ScriptValue& result);
    static void convertValue(VALUE value, Vector<String>& result);
    static void convertValue(VALUE value, RefPtr<SerializedScriptValue>& result);
    static void convertValue(VALUE value, RefPtr<DOMWindow>& result);
    static void convertValue(VALUE value, RefPtr<EventTarget>& result);
    static void convertValue(VALUE value, RefPtr<Node>& result);
    static void convertValue(VALUE value, RefPtr<Storage>& result);
    static void convertValue(VALUE value, MessagePortArray& result);
#if ENABLE(VIDEO_TRACK)
    static void convertValue(VALUE value, RefPtr<TrackBase>& result);
#endif
    static void convertValue(VALUE value, HashSet<AtomicString>& result);
    static void convertValue(VALUE value, ArrayValue& result);
    static void convertValue(VALUE value, RefPtr<Uint8Array>& result);
#if ENABLE(ENCRYPTED_MEDIA)
    static void convertValue(VALUE value, RefPtr<MediaKeyError>& result);
#endif

private:
    VALUE m_hash;
};

template <typename Result>
bool RBDictionary::get(const char* propertyName, Result& result) const
{
    // FIXME: Do we need to convert from camelCase to underscore_case?
    VALUE value = rb_hash_aref(m_hash, ID2SYM(rb_intern(propertyName)));
    if (NIL_P(value))
        return false;

    convertValue(value, result);
    return true;
}

} // namespace WebCore

#endif // RBDictionary_h
