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

#if ENABLE(SQL_DATABASE)

#include "RBSQLResultSetRowList.h"

#include "ExceptionCode.h"
#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include <Ruby/ruby.h>
#include <wtf/text/CString.h>

namespace WebCore {

VALUE RBSQLResultSetRowList::item(VALUE self, VALUE rbIndex)
{
    SQLResultSetRowList* list = impl<SQLResultSetRowList>(self);
    int index = NUM2INT(rbIndex);
    if (index < 0 || (unsigned) index >= list->length()) {
        rbDOMRaiseError(INDEX_SIZE_ERR);
        return Qnil;
    }

    VALUE hash = rb_hash_new();

    unsigned numColumns = list->columnNames().size();
    unsigned valuesIndex = index * numColumns;
    for (unsigned i = 0; i < numColumns; i++) {
        const SQLValue& value = list->values()[valuesIndex + i];

        VALUE rbValue;

        switch (value.type()) {
        case SQLValue::StringValue:
            rbValue = rb_str_new2(value.string().utf8().data());
            break;
        case SQLValue::NullValue:
            rbValue = Qnil;
            break;
        case SQLValue::NumberValue:
            rbValue = rb_float_new(value.number());
            break;
        default:
            ASSERT_NOT_REACHED();
        }

        rb_hash_aset(hash, ID2SYM(rb_intern(list->columnNames()[i].utf8().data())), rbValue);
    }

    return hash;
}

#endif // ENABLE(SQL_DATABASE)

} // namespace WebCore
