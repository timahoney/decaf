/*
 * Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
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

#ifndef RBConverters_h
#define RBConverters_h

#include "KURL.h"
#include "RBMessagePort.h"
#include "RBObject.h"
#include "ScriptValue.h"
#include "SerializedScriptValue.h"
#include "SVGStaticPropertyTearOff.h"
#include "SVGStaticPropertyWithParentTearOff.h"
#include <Ruby/ruby.h>
#include <wtf/GetPtr.h>
#include <wtf/text/CString.h>
#include <wtf/Vector.h>

namespace WebCore {

#define IS_RB_KIND(object, klass) RTEST(rb_obj_is_kind_of(object, RB##klass::rubyClass()))
#define IS_RB_STRING(obj) (TYPE(obj) == T_STRING)
#define IS_RB_INT(obj) (TYPE(obj) == T_FIXNUM || TYPE(obj) == T_BIGNUM)
#define IS_RB_FLOAT(obj) (TYPE(obj) == T_FLOAT)
#define IS_RB_NUM(obj) RTEST(rb_obj_is_kind_of(obj, rb_cNumeric))
#define IS_RB_BOOL(obj) (TYPE(obj) == T_FALSE || TYPE(obj) == T_TRUE)
#define IS_RB_HASH(obj) (TYPE(obj) == T_HASH)
#define IS_RB_ARRAY(obj) (TYPE(obj) == T_ARRAY)
#define IS_RB_SYMBOL(obj) (TYPE(obj) == T_SYMBOL)

inline String rbToString(VALUE object)
{
    VALUE rbString = rb_funcall(object, rb_intern("to_s"), 0);
    return StringValueCStr(rbString);
}

inline VALUE toRB(float value) { return rb_float_new(value); }
inline VALUE toRB(double value) { return rb_float_new(value); }
inline VALUE toRB(int value) { return INT2NUM(value); }
inline VALUE toRB(short value) { return INT2NUM(value); }
inline VALUE toRB(long value) { return LONG2NUM(value); }
inline VALUE toRB(long long value) { return LL2NUM(value); }
inline VALUE toRB(unsigned value) { return UINT2NUM(value); }
inline VALUE toRB(unsigned short value) { return UINT2NUM(value); }
inline VALUE toRB(unsigned long value) { return ULONG2NUM(value); }
inline VALUE toRB(unsigned long long value) { return ULL2NUM(value); }
inline VALUE toRB(bool value) { return value ? Qtrue : Qfalse; }
inline VALUE toRB(const KURL& value) { return rb_str_new2(value.string().utf8().data()); }

// This is a very hacky way of having multiple toRBs that take doubles.
// It makes it nicer for the code generator to use this instead of
// something like toRBDate(double).
enum RBConvertDate { Date };
inline VALUE toRB(double value, RBConvertDate) { return rb_time_new(value / 1000, 0); }

inline VALUE toRB(const String& value, bool convertToEmptyString = true)
{ 
    if (!value.isNull())
        return rb_str_new2(value.utf8().data());
    if (convertToEmptyString)
        return rb_str_new2("");
    return Qnil;
}

template <typename PropertyType, typename ContextElement>
VALUE toRB(PassRefPtr<SVGStaticPropertyTearOff<ContextElement, PropertyType> > impl)
{
    RefPtr<SVGPropertyTearOff<PropertyType> > nonStaticRef = impl;
    return toRB(nonStaticRef.release());
}

template <typename PropertyType, typename ContextElement>
VALUE toRB(PassRefPtr<SVGStaticPropertyWithParentTearOff<ContextElement, PropertyType> > impl)
{
    RefPtr<SVGPropertyTearOff<PropertyType> > nonStaticRef = impl;
    return toRB(nonStaticRef.release());
}

template <class T>
struct RBValueTraits {
    static inline VALUE arrayRBValue(const T& value)
    {
        return toRB(WTF::getPtr(value));
    }
};

template<>
struct RBValueTraits<String> {
    static inline VALUE arrayRBValue(const String& value)
    {
        return toRB(value);
    }
};

template<>
struct RBValueTraits<float> {
    static inline VALUE arrayRBValue(const float& value)
    {
        return toRB(value);
    }
};

template<>
struct RBValueTraits<unsigned long> {
    static inline VALUE arrayRBValue(const unsigned long& value)
    {
        return toRB(value);
    }
};

template <class T>
VALUE toRB(const Vector<T>* vector)
{
    VALUE array = rb_ary_new2(vector->size());
    typename Vector<T>::const_iterator end = vector->end();
    typedef RBValueTraits<T> TraitsType;
    for (typename Vector<T>::const_iterator iter = vector->begin(); iter != end; ++iter) {
        VALUE entry = TraitsType::arrayRBValue(*iter);
        rb_ary_push(array, entry);
    }

    return array;
}

template <class T>
VALUE toRB(const Vector<T>& vector)
{
    const Vector<T>* ptr = &vector;
    return toRB(ptr);
}

template<class T> struct RBNativeValueTraits;

template<>
struct RBNativeValueTraits<String> {
    static inline bool nativeValue(VALUE rbValue, String& indexedValue)
    {
        if (!IS_RB_STRING(rbValue))
            return false;

        indexedValue = StringValueCStr(rbValue);
        return true;
    }
};

template<>
struct RBNativeValueTraits<unsigned long> {
    static inline bool nativeValue(VALUE rbValue, unsigned long& indexedValue)
    {
        if (!IS_RB_NUM(rbValue))
            return false;

        indexedValue = NUM2ULONG(rbValue);
        return true;
    }
};

template<>
struct RBNativeValueTraits<unsigned> {
    static inline bool nativeValue(VALUE rbValue, unsigned& indexedValue)
    {
        if (!IS_RB_NUM(rbValue))
            return false;
        
        indexedValue = NUM2UINT(rbValue);
        return true;
    }
};

template<>
struct RBNativeValueTraits<int> {
    static inline bool nativeValue(VALUE rbValue, int& indexedValue)
    {
        if (!IS_RB_NUM(rbValue))
            return false;
        
        indexedValue = NUM2INT(rbValue);
        return true;
    }
};

template<>
struct RBNativeValueTraits<float> {
    static inline bool nativeValue(VALUE rbValue, float& indexedValue)
    {
        if (!IS_RB_NUM(rbValue))
            return false;

        indexedValue = NUM2DBL(rbValue);
        return true;
    }
};

template<>
struct RBNativeValueTraits<RefPtr<MessagePort> > {
    static inline bool nativeValue(VALUE rbValue, RefPtr<MessagePort>& indexedValue)
    {
        if (!IS_RB_KIND(rbValue, MessagePort))
            return false;
        
        indexedValue = impl<MessagePort>(rbValue);
        return true;
    }
};

template <class T, size_t inlineCapacity = 0>
Vector<T> rbToNativeArray(VALUE rbArray)
{
    VALUE rbLength = rb_funcall(rbArray, rb_intern("length"), 0);
    unsigned length = NUM2UINT(rbLength);
    Vector<T, inlineCapacity> result;

    typedef RBNativeValueTraits<T> TraitsType;
    for (unsigned i = 0; i < length; ++i) {
        T indexValue;
        if (!TraitsType::nativeValue(rb_ary_entry(rbArray, i), indexValue))
            return Vector<T>();
        result.append(indexValue);
    }

    return result;
}

inline String rbStringOrNullString(VALUE rbString) { return NIL_P(rbString) ? String() : StringValueCStr(rbString); }
inline String rbStringOrUndefined(VALUE rbString) { return NIL_P(rbString) ? "undefined" : StringValueCStr(rbString); }

inline double rbToDate(VALUE rbDate) {
    if (NIL_P(rbDate))
        return 0;
    if (IS_RB_NUM(rbDate))
        return NUM2DBL(rbDate);
    if (CLASS_OF(rbDate) == rb_cTime)
        return rb_time_timeval(rbDate).tv_sec;
    
    return 0;
}

} // namespace WebCore

#endif
