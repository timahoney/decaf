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

#ifndef RBSerializationDelegate_h
#define RBSerializationDelegate_h

#include "SerializationDelegate.h"
#include <wtf/OwnPtr.h>

namespace WebCore {

class JSDOMGlobalObject;

class RBSerializationDelegate : public SerializationDelegate {
public:
    static PassOwnPtr<RBSerializationDelegate> create()
    {
        return adoptPtr(new RBSerializationDelegate());
    }

    virtual ~RBSerializationDelegate();

    virtual bool shouldTerminate();
    virtual unsigned ticksUntilNextCheck();
    virtual bool didTimeOut();
    virtual void throwStackOverflow();
    virtual void throwInterruptedException();
    virtual void throwValidationError();

    virtual void maybeThrowExceptionIfSerializationFailed(SerializationReturnCode);

    virtual SerializationTag tagForObject(const ScriptValue&);

    virtual ScriptValue toScriptValue(MessagePort*);
    virtual ScriptValue toScriptValue(ArrayBuffer*);
    virtual ScriptValue toScriptValue(File*);
    virtual ScriptValue toScriptValue(FileList*);
    virtual ScriptValue toScriptValue(ImageData*);
    virtual ScriptValue toScriptValue(Blob*);
    virtual ScriptValue toScriptValue(DataView*);
    virtual ScriptValue toScriptValue(Int8Array*);
    virtual ScriptValue toScriptValue(Uint8Array*);
    virtual ScriptValue toScriptValue(Uint8ClampedArray*);
    virtual ScriptValue toScriptValue(Int16Array*);
    virtual ScriptValue toScriptValue(Uint16Array*);
    virtual ScriptValue toScriptValue(Int32Array*);
    virtual ScriptValue toScriptValue(Uint32Array*);
    virtual ScriptValue toScriptValue(Float32Array*);
    virtual ScriptValue toScriptValue(Float64Array*);
    virtual ScriptValue toScriptValue(double d);
    virtual ScriptValue toScriptValue(int64_t i);
    virtual ScriptValue toScriptValue(bool value);
    virtual ScriptValue toScriptValue(const String&);
    virtual ScriptValue toRegExpScriptValue(const String& flags, const String& pattern);
    virtual ScriptValue toDateScriptValue(double time);

    virtual ScriptValue toBooleanObjectScriptValue(bool);
    virtual ScriptValue toNumberObjectScriptValue(double);
    virtual ScriptValue toStringObjectScriptValue(const String&);

    virtual ScriptValue null();
    virtual ScriptValue undefined();
    virtual ScriptValue emptyString();

    virtual bool isArray(const ScriptValue&);
    virtual bool isDate(const ScriptValue&);
    virtual double asDateValue(const ScriptValue&);
    virtual double asNumber(const ScriptValue&);
    virtual String asString(const ScriptValue&);
    virtual File* toFile(const ScriptValue&);
    virtual FileList* toFileList(const ScriptValue&);
    virtual Blob* toBlob(const ScriptValue&);
    virtual ImageData* toImageData(const ScriptValue&);
    virtual ArrayBuffer* toArrayBuffer(const ScriptValue&);
    virtual ArrayBufferView* toArrayBufferView(const ScriptValue&);

    virtual bool isRegExpGlobal(const ScriptValue&);
    virtual bool isRegExpIgnoreCase(const ScriptValue&);
    virtual bool isRegExpMultiline(const ScriptValue&);
    virtual String asRegExpPattern(const ScriptValue&);

    virtual void setArrayElement(const ScriptValue& array, unsigned index, const ScriptValue& value);
    virtual void setObjectProperty(const ScriptValue& object, const String& name, const ScriptValue& value);

    virtual void getOwnPropertyNames(const ScriptValue&, Vector<String>&);
    virtual void getOwnNonIndexPropertyNames(const ScriptValue&, Vector<String>&);
    virtual ScriptValue objectAtIndex(const ScriptValue&, unsigned i);
    virtual unsigned arrayLength(const ScriptValue&);
    virtual ScriptValue getProperty(const ScriptValue&, const String& name);
    virtual bool isFinalObject(const ScriptValue&);

    virtual ScriptValue constructEmptyArray(unsigned length);
    virtual ScriptValue constructEmptyObject();

protected:
    RBSerializationDelegate();
};

} // namespace WebCore

#endif // RBSerializationDelegate