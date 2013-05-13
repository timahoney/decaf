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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "config.h"
#include "RBSerializationDelegate.h"

#include "ExceptionCode.h"
#include "RBArrayBuffer.h"
#include "RBArrayBufferView.h"
#include "RBBlob.h"
#include "RBDataView.h"
#include "RBDOMBinding.h"
#include "RBExceptionHandler.h"
#include "RBFile.h"
#include "RBFileList.h"
#include "RBFloat32Array.h"
#include "RBFloat64Array.h"
#include "RBImageData.h"
#include "RBInt16Array.h"
#include "RBInt32Array.h"
#include "RBInt8Array.h"
#include "RBMessagePort.h"
#include "RBNavigator.h"
#include "RBScriptState.h"
#include "RBScriptValue.h"
#include "RBUint16Array.h"
#include "RBUint32Array.h"
#include "RBUint8Array.h"
#include "RBUint8ClampedArray.h"
#include <Ruby/ruby.h>

using namespace RB;

namespace WebCore {

RBSerializationDelegate::RBSerializationDelegate()
    : SerializationDelegate(RBScriptType)
{
}

RBSerializationDelegate::~RBSerializationDelegate()
{
}

bool RBSerializationDelegate::shouldTerminate()
{
    return !NIL_P(rb_errinfo());
}

unsigned RBSerializationDelegate::ticksUntilNextCheck()
{
    return UINT32_MAX;
}

bool RBSerializationDelegate::didTimeOut()
{
    return false;
}

void RBSerializationDelegate::throwStackOverflow()
{
    rb_raise(rb_eSysStackError, "Maximum call stack size exceeded.");
}

void RBSerializationDelegate::throwInterruptedException()
{
    rb_raise(rb_eInterrupt, "Execution was interrupted.");
}

void RBSerializationDelegate::throwValidationError()
{
    rb_raise(rb_eTypeError, "Unable to deserialize data.");
}

void RBSerializationDelegate::maybeThrowExceptionIfSerializationFailed(SerializationReturnCode code)
{
    if (code == SuccessfullyCompleted)
        return;
    
    switch (code) {
    case StackOverflowError:
        throwStackOverflow();
        break;
    case InterruptedExecutionError:
        throwInterruptedException();
        break;
    case ValidationError:
        throwValidationError();
        break;
    case DataCloneError:
        setDOMException(DATA_CLONE_ERR);
        break;
    case ExistingExceptionError:
        break;
    case UnspecifiedError:
        break;
    default:
        ASSERT_NOT_REACHED();
    }
}

ScriptValue RBSerializationDelegate::toScriptValue(MessagePort* messagePort)
{
    return toRB(messagePort);
}

ScriptValue RBSerializationDelegate::toScriptValue(ArrayBuffer* arrayBuffer)
{
    return toRB(arrayBuffer);
}

ScriptValue RBSerializationDelegate::toScriptValue(DataView* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(Int8Array* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(Uint8Array* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(Uint8ClampedArray* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(Int16Array* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(Uint16Array* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(Int32Array* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(Uint32Array* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(Float32Array* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(Float64Array* array)
{
    return toRB(array);
}

ScriptValue RBSerializationDelegate::toScriptValue(File* file)
{
    return toRB(file);
}

ScriptValue RBSerializationDelegate::toScriptValue(FileList* fileList)
{
    return toRB(fileList);
}

ScriptValue RBSerializationDelegate::toScriptValue(ImageData* imageData)
{
    return toRB(imageData);
}

ScriptValue RBSerializationDelegate::toScriptValue(Blob* blob)
{
    return toRB(blob);
}

ScriptValue RBSerializationDelegate::toScriptValue(const String& string)
{
    return toRB(string);
}

ScriptValue RBSerializationDelegate::null()
{
    return Qnil;
}

ScriptValue RBSerializationDelegate::undefined()
{
    // Should this be Qundef? Ruby's undefined is different than JavaScript.
    return Qnil;
}

ScriptValue RBSerializationDelegate::emptyString()
{
    return toScriptValue(::emptyString());
}

ScriptValue RBSerializationDelegate::toScriptValue(double d)
{
    return toRB(d);
}

ScriptValue RBSerializationDelegate::toScriptValue(int64_t i)
{
    return toRB(i);
}

ScriptValue RBSerializationDelegate::toScriptValue(bool value)
{
    return toRB(value);
}

ScriptValue RBSerializationDelegate::toRegExpScriptValue(const String& flags, const String& pattern)
{
    int options = 0;
    if (flags.contains('i'))
        options |= FIX2INT(rb_const_get(rb_cRegexp, rb_intern("IGNORECASE")));
    if (flags.contains('m'))
        options |= FIX2INT(rb_const_get(rb_cRegexp, rb_intern("MULTILINE")));
    VALUE value = rb_reg_new(pattern.utf8().data(), pattern.length(), options);
    return value;
}

ScriptValue RBSerializationDelegate::toDateScriptValue(double time)
{
    VALUE value = toRB(time, Date);
    return value;
}

ScriptValue RBSerializationDelegate::toBooleanObjectScriptValue(bool value)
{
    return toScriptValue(value);
}

ScriptValue RBSerializationDelegate::toNumberObjectScriptValue(double d)
{
    return toScriptValue(d);
}

ScriptValue RBSerializationDelegate::toStringObjectScriptValue(const String& value)
{
    return toScriptValue(value);
}

bool RBSerializationDelegate::isArray(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return IS_RB_ARRAY(value);
}

unsigned RBSerializationDelegate::arrayLength(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    ASSERT(IS_RB_ARRAY(value));
    return RARRAY_LEN(value);
}

ScriptValue RBSerializationDelegate::getProperty(const ScriptValue& scriptValue, const String& name)
{
    // FIXME: Should we add instance variables to this too?
    // Right now, this just works with hashes.
    
    VALUE value = scriptValue.rbValue();
    ID id = rb_intern(name.utf8().data());
    VALUE result = rb_hash_aref(value, ID2SYM(id));

    // If the original key wasn't a symbol, try a string.
    if (NIL_P(result)) {
        VALUE string = rb_id2str(id);
        result = rb_hash_aref(value, string);

        // If the origianl key was neither a string nor a symbol, try a number.
        if (NIL_P(result)) {
            VALUE number = rb_funcall(string, rb_intern("to_i"), 0);
            result = rb_hash_aref(value, number);
        }
    }
    
    if (NIL_P(result))
        return ScriptValue();
    return result;
}

bool RBSerializationDelegate::isDate(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return rb_obj_is_kind_of(value, rb_cTime);
}

double RBSerializationDelegate::asDateValue(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return rbToDate(value);
}

SerializationTag RBSerializationDelegate::tagForObject(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    if (IS_RB_BOOL(value))
        return RTEST(value) ? TrueObjectTag : FalseObjectTag;
    if (IS_RB_STRING(value))
        return StringObjectTag;
    if (IS_RB_NUM(value))
        return NumberObjectTag;
    if (IS_RB_KIND(value, File))
        return FileTag;
    if (IS_RB_KIND(value, FileList))
        return FileListTag;
    if (IS_RB_KIND(value, Blob))
        return BlobTag;
    if (IS_RB_KIND(value, ImageData))
        return ImageDataTag;
    if (TYPE(value) == T_REGEXP)
        return RegExpTag;
    if (IS_RB_KIND(value, MessagePort))
        return MessagePortReferenceTag;
    if (IS_RB_KIND(value, ArrayBuffer))
        return ArrayBufferTag;
    if (IS_RB_KIND(value, ArrayBufferView))
        return ArrayBufferViewTag;
    return ErrorTag;
}

double RBSerializationDelegate::asNumber(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return NUM2DBL(value);
}

String RBSerializationDelegate::asString(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return rbToString(value);
}

File* RBSerializationDelegate::toFile(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return impl<File>(value);
}

FileList* RBSerializationDelegate::toFileList(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return impl<FileList>(value);
}

Blob* RBSerializationDelegate::toBlob(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return impl<Blob>(value);
}

ImageData* RBSerializationDelegate::toImageData(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return impl<ImageData>(value);
}

bool RBSerializationDelegate::isRegExpGlobal(const ScriptValue&)
{
    // Ruby regular expressions don't have a global attribute.
    // This could go either way, but we can just assume that 
    // regular expressions from Ruby are global.
    return true;
}

bool RBSerializationDelegate::isRegExpIgnoreCase(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    int options = rb_reg_options(value);
    return options & FIX2INT(rb_const_get(rb_cRegexp, rb_intern("IGNORECASE")));
}

bool RBSerializationDelegate::isRegExpMultiline(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    int options = rb_reg_options(value);
    return options & FIX2INT(rb_const_get(rb_cRegexp, rb_intern("MULTILINE")));
}

String RBSerializationDelegate::asRegExpPattern(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    VALUE source = rb_funcall(value, rb_intern("source"), 0);
    return rbToString(source);
}

ArrayBuffer* RBSerializationDelegate::toArrayBuffer(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return impl<ArrayBuffer>(value);
}

ArrayBufferView* RBSerializationDelegate::toArrayBufferView(const ScriptValue& scriptValue)
{
    VALUE value = scriptValue.rbValue();
    return impl<ArrayBufferView>(value);
}

void RBSerializationDelegate::setArrayElement(const ScriptValue& arrayScriptValue, unsigned index, const ScriptValue& elementScriptValue)
{
    VALUE array = arrayScriptValue.rbValue();
    VALUE element = elementScriptValue.rbValue();
    ASSERT(IS_RB_ARRAY(array));
    rb_ary_store(array, index, element);
}

void RBSerializationDelegate::setObjectProperty(const ScriptValue& objectScriptValue, const String& propertyName, const ScriptValue& propertyScriptValue)
{
    VALUE hash = static_cast<RBScriptValue*>(objectScriptValue.delegate())->rbValue();
    VALUE value = static_cast<RBScriptValue*>(propertyScriptValue.delegate())->rbValue();
    VALUE key = ID2SYM(rb_intern(propertyName.utf8().data()));
    ASSERT(IS_RB_HASH(hash));
    rb_hash_aset(hash, key, value);
}

static int appendKey(VALUE key, VALUE, Vector<String>* strings)
{
    strings->append(rbToString(key));
    return ST_CONTINUE;
}

void RBSerializationDelegate::getOwnPropertyNames(const ScriptValue& scriptValue, Vector<String>& strings)
{
    VALUE hash = scriptValue.rbValue();
    rb_hash_foreach(hash, (int (*)(ANYARGS)) (&appendKey), (VALUE) &strings);
}

void RBSerializationDelegate::getOwnNonIndexPropertyNames(const ScriptValue& scriptValue, Vector<String>& strings)
{
    // This is called on arrays, and we don't want to serialize any other data about arrays.
    UNUSED_PARAM(scriptValue);
    UNUSED_PARAM(strings);
}

ScriptValue RBSerializationDelegate::objectAtIndex(const ScriptValue& scriptValue, unsigned index)
{
    VALUE value = scriptValue.rbValue();
    ASSERT(IS_RB_ARRAY(value));
    VALUE objectAtIndex = rb_ary_entry(value, index);
    return objectAtIndex;
}

bool RBSerializationDelegate::isFinalObject(const ScriptValue& scriptValue)
{
    // FIXME: Is this correct?
    VALUE value = scriptValue.rbValue();
    return IS_RB_HASH(value);
}

ScriptValue RBSerializationDelegate::constructEmptyArray(unsigned length)
{
    return rb_ary_new2(length);
}

ScriptValue RBSerializationDelegate::constructEmptyObject()
{
    return rb_hash_new();
}

} // namespace WebCore
