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

#include "config.h"
#include "RBExceptionHandler.h"

#include "ExceptionCodeDescription.h"
#include "ExceptionHeaders.h"
#include "ExceptionInterfaces.h"

using namespace WebCore;

namespace RB {

#define TRY_TO_CREATE_EXCEPTION(interfaceName) \
    case interfaceName##Type: \
        errorObject = toRB(interfaceName::create(description)); \
        break;

void setDOMException(ExceptionCode ec)
{
    if (!ec)
        return;
    
    ExceptionCodeDescription description = ExceptionCodeDescription(ec);

    // FIXME: Remove this console printing.
    printf("--RUBY EXCEPTION--\n");
    printf("Code:\t\t%d\n", description.code);
    printf("Type:\t\t%s : %s\n", description.typeName, description.name);
    printf("Message:\t%s\n", description.description);

    VALUE errorObject;
    switch (description.type) {
        DOM_EXCEPTION_INTERFACES_FOR_EACH(TRY_TO_CREATE_EXCEPTION)
    }

    rb_exc_raise(errorObject);
}

} // namespace RB
