/*
 * Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RBCallHelpers_h
#define RBCallHelpers_h

#include <Ruby/ruby.h>

namespace RB {

// Here are a few helpers to call a Ruby function and protect it.
// If there was an exception, then it will be stored in the 'exception' argument.
VALUE callFunction(VALUE object, const char* functionName, int argc, VALUE* argv, VALUE* exception = 0);

// It makes sense to have a variadic function for these, but defining multiple
// functions allows us to put the optional exception value at the end.
// If you ever need to use more than four arguments, then make another function.
VALUE callFunction(VALUE object, const char* functionName, VALUE* exception = 0);
VALUE callFunction(VALUE object, const char* functionName, VALUE arg1, VALUE* exception = 0);
VALUE callFunction(VALUE object, const char* functionName, VALUE arg1, VALUE arg2, VALUE* exception = 0);
VALUE callFunction(VALUE object, const char* functionName, VALUE arg1, VALUE arg2, VALUE arg3, VALUE* exception = 0);
VALUE callFunction(VALUE object, const char* functionName, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4, VALUE* exception = 0);

} // namespace RB

#endif // RBCallHelpers_h
