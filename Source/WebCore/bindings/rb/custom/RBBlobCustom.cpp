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
#include "RBBlob.h"

#include "RBArrayBuffer.h"
#include "RBArrayBufferView.h"
#include "RBBlobCustom.h"
#include "RBConverters.h"
#include "RBFile.h"
#include "RBScriptState.h"
#include "WebKitBlobBuilder.h"
#include <wtf/Assertions.h>

namespace WebCore {

VALUE RBBlob::rb_new(int argc, VALUE* argv, VALUE)
{
    VALUE partsRB, propertiesRB;
    rb_scan_args(argc, argv, "02", &partsRB, &propertiesRB);

    RBScriptState* state = RBScriptState::current();
    ScriptExecutionContext* context = state->scriptExecutionContext();
    if (!context) {
        rb_raise(rb_eRuntimeError, "Blob constructor associated document is unavailable");
        return Qnil;
    }

    if (NIL_P(partsRB)) {
        RefPtr<Blob> blob = Blob::create();
        return toRB(blob.release());
    }

    if (!IS_RB_ARRAY(partsRB)) {
        rb_raise(rb_eTypeError, "First argument of the Blob constructor should be of type Array");
        return Qnil;
    }

    String type;
    String endings = ASCIILiteral("transparent");

    if (!NIL_P(propertiesRB)) {
        if (!IS_RB_HASH(propertiesRB)) {
            rb_raise(rb_eTypeError, "Second argument of the Blob constructor should be of type Hash");
            return Qnil;
        }

        // Attempt to get the endings property and validate it.
        VALUE endingsRB = rb_hash_aref(propertiesRB, ID2SYM(rb_intern("endings")));
        if (!NIL_P(endingsRB)) {
            endings = StringValueCStr(endingsRB);
            if (endings != "transparent" && endings != "native") {
                rb_raise(rb_eArgError, "The endings property must be either \"transparent\" or \"native\"");
                return Qnil;
            }
        }

        // Attempt to get the type property.
        VALUE typeRB = rb_hash_aref(propertiesRB, ID2SYM(rb_intern("type")));
        if (!NIL_P(typeRB)) {
            type = StringValueCStr(typeRB);
            if (!type.containsOnlyASCII()) {
                rb_raise(rb_eSyntaxError, "The type property must consist of only ASCII characters");
                return Qnil;
            }
            type.makeLower();
        }
    }

    ASSERT(endings == "transparent" || endings == "native");

    BlobBuilder blobBuilder;
    unsigned length = RARRAY_LEN(partsRB);
    for (unsigned i = 0; i < length; ++i) {
        VALUE item = rb_ary_entry(partsRB, i);
#if ENABLE(BLOB)
        if (IS_RB_KIND(item, ArrayBuffer))
            blobBuilder.append(impl<ArrayBuffer>(item));
        else if (IS_RB_KIND(item, ArrayBufferView))
            blobBuilder.append(impl<ArrayBufferView>(item));
        else
#endif
        if (IS_RB_KIND(item, Blob))
            blobBuilder.append(impl<Blob>(item));
        else {
            String string = rbToString(item);
            blobBuilder.append(string, endings);
        }
    }

    RefPtr<Blob> blob = blobBuilder.getBlob(type);
    return toRB(blob.release());
}

VALUE toRB(Blob* impl)
{
    if (!impl)
        return Qnil;

    if (impl->isFile())
        return toRB(static_cast<File*>(impl));

    return toRB(RBBlob::rubyClass(), impl);
}

VALUE RBBlobCustom::marshal_load(VALUE, VALUE data)
{
    VALUE rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));
    VALUE values = rb_funcall(rb_mMarshal, rb_intern("load"), 1, data);
    
    KURL url = KURL(KURL(), rbToString(rb_ary_entry(values, 0)));
    String type = rbToString(rb_ary_entry(values, 1));
    unsigned long long size = NUM2ULL(rb_ary_entry(values, 2));
    RefPtr<Blob> blob = Blob::create(url, type, size);
    return toRB(blob.release());
}

VALUE RBBlobCustom::marshal_dump(VALUE self, VALUE)
{
    Blob* blob = impl<Blob>(self);
    VALUE array = rb_ary_new2(3);
    rb_ary_push(array, toRB(blob->url()));
    rb_ary_push(array, toRB(blob->type()));
    rb_ary_push(array, toRB(blob->size()));
    
    VALUE rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));
    VALUE data = rb_funcall(rb_mMarshal, rb_intern("dump"), 1, array);
    return data;
}

void RBBlobCustom::Init_BlobCustom()
{
    rb_define_method(RBBlob::rubyClass(), "_dump", RUBY_METHOD_FUNC(&RBBlobCustom::marshal_dump), 1);
    rb_define_module_function(RBBlob::rubyClass(), "_load", RUBY_METHOD_FUNC(&RBBlobCustom::marshal_load), 1);
}
    
}
