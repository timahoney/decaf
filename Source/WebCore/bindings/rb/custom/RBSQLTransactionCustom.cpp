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

#include "RBSQLTransaction.h"

#include "RBConverters.h"
#include "RBExceptionHandler.h"
#include "RBScriptState.h"
#include "RBSQLStatementCallback.h"
#include "RBSQLStatementErrorCallback.h"

namespace WebCore {

VALUE RBSQLTransaction::execute_sql(int argc, VALUE* argv, VALUE self)
{
    SQLTransaction* selfImpl = impl<SQLTransaction>(self);
    RBScriptState* state = RBScriptState::current();
    VALUE sqlStatementRB, argumentsRB, callbackRB, errorCallbackRB;
    rb_scan_args(argc, argv, "22", &sqlStatementRB, &argumentsRB, &callbackRB, &errorCallbackRB);
    if (rb_block_given_p()) {
        errorCallbackRB = callbackRB;
        callbackRB = rb_block_proc();
    }

    if (argc == 0) {
        rbDOMRaiseError(SYNTAX_ERR);
        return Qnil;
    }

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

    RefPtr<SQLStatementCallback> callback;
    if (!NIL_P(callbackRB)) {
        if (CLASS_OF(callbackRB) != rb_cProc) {
            rbDOMRaiseError(TYPE_MISMATCH_ERR);
            return Qnil;
        }

        callback = RBSQLStatementCallback::create(callbackRB, state->scriptExecutionContext());
    }

    RefPtr<SQLStatementErrorCallback> errorCallback;
    if (!NIL_P(errorCallbackRB)) {
        if (CLASS_OF(errorCallbackRB) != rb_cProc) {
            rbDOMRaiseError(TYPE_MISMATCH_ERR);
            return Qnil;
        }

        errorCallback = RBSQLStatementErrorCallback::create(errorCallbackRB, state->scriptExecutionContext());
    }

    ExceptionCode ec = 0;
    selfImpl->executeSQL(sqlStatement, sqlValues, callback.release(), errorCallback.release(), ec);
    rbDOMRaiseError(ec);

    return Qnil;
}

#endif // ENABLE(SQL_DATABASE)

} // namespace WebCore
