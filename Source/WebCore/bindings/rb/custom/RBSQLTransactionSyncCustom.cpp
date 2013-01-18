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

#include "RBSQLTransactionSync.h"

#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include "RBSQLResultSet.h"
#include "SQLValue.h"

namespace WebCore {

VALUE RBSQLTransactionSync::execute_sql(VALUE self, VALUE sqlStatementRB, VALUE argumentsRB)
{
    SQLTransactionSync* selfImpl = impl<SQLTransactionSync>(self);
    String sqlStatement = StringValueCStr(sqlStatementRB);

    // Now assemble the list of SQL arguments
    Vector<SQLValue> sqlValues;
    if (!NIL_P(argumentsRB)) {
        if (!IS_RB_ARRAY(argumentsRB)) {
            rbDOMRaiseError(TYPE_MISMATCH_ERR);
            return Qnil;
        }        

        unsigned length = RARRAY_LEN(argumentsRB);
        for (unsigned i = 0; i < length; ++i) {
            VALUE value = rb_ary_entry(argumentsRB, i);
            if (NIL_P(value))
                sqlValues.append(SQLValue());
            else if (IS_RB_NUM(value))
                sqlValues.append(NUM2DBL(value));
            else
                sqlValues.append(rbToString(value));
        }
    }

    ExceptionCode ec = 0;
    RefPtr<SQLResultSet> result = selfImpl->executeSQL(sqlStatement, sqlValues, ec);
    rbDOMRaiseError(ec);

    return toRB(result.release());
}


#endif // ENABLE(SQL_DATABASE)

} // namespace WebCore
