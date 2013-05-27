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
#include "RBScriptCallStackFactory.h"

#include "InspectorInstrumentation.h"
#include "RBCallHelpers.h"
#include "RBScriptState.h"
#include "RBScriptValue.h"
#include "ScriptArguments.h"
#include "ScriptCallFrame.h"
#include "ScriptCallStack.h"
#include "ScriptValue.h"
#include <wtf/text/WTFString.h>

using namespace RB;

namespace WebCore {

class ScriptExecutionContext;

static void createFramesFromBacktrace(VALUE backtrace, size_t maxStackSize, Vector<ScriptCallFrame>& frames)
{
    VALUE regex = rb_reg_new_str(rb_str_new2("^(.*?):(\\d+):in `(.+)'$"), 0);
    for (int i = 0; i < RARRAY_LEN(backtrace) && frames.size() < maxStackSize; i++) {
        VALUE rbBacktraceString = rb_ary_entry(backtrace, i);
        VALUE match = rb_funcall(regex, rb_intern("match"), 1, rbBacktraceString);
        
        VALUE rbFile = rb_funcall(match, rb_intern("[]"), 1, INT2FIX(1));
        String file;
        if (!NIL_P(rbFile))
            file = StringValueCStr(rbFile);
        
        VALUE lineNumberString = rb_funcall(match, rb_intern("[]"), 1, INT2FIX(2));
        VALUE lineNumberRB = rb_funcall(lineNumberString, rb_intern("to_i"), 0);
        int lineNumber = NUM2INT(lineNumberRB);
            
        VALUE rbFunction = rb_funcall(match, rb_intern("[]"), 1, INT2FIX(3));
        String functionName;
        if (!NIL_P(rbFunction))
            functionName = StringValueCStr(rbFunction);
        else if (!frames.isEmpty())
            break;
        
        frames.append(ScriptCallFrame(functionName, file, lineNumber));
        
        if (NIL_P(rbFunction))
            break;
    }
}

PassRefPtr<ScriptCallStack> RBScriptCallStackFactory::createScriptCallStack(size_t maxStackSize, bool emptyIsAllowed)
{
    VALUE backtrace = rb_funcall(rb_thread_current(), rb_intern("backtrace"), 0);
    Vector<ScriptCallFrame> frames;
    createFramesFromBacktrace(backtrace, maxStackSize, frames);

    if (frames.isEmpty() && !emptyIsAllowed) {
        // No frames found. It may happen in the case where
        // a bound function is called from native code for example.
        // Fallback to setting lineNumber to 0, and source and function name to "undefined".
        frames.append(ScriptCallFrame("undefined", "undefined", 0));
    }
    
    return ScriptCallStack::create(frames);
}

PassRefPtr<ScriptCallStack> RBScriptCallStackFactory::createScriptCallStack(RBScriptState* state, size_t maxStackSize)
{
    VALUE backtrace = callFunction(state->binding(), "eval", rb_str_new2("caller"));
    Vector<ScriptCallFrame> frames;
    createFramesFromBacktrace(backtrace, maxStackSize, frames);
    return ScriptCallStack::create(frames);
}

PassRefPtr<ScriptCallStack> RBScriptCallStackFactory::createScriptCallStackForConsole(RBScriptState* state)
{
    size_t maxStackSize = 1;

    if (InspectorInstrumentation::hasFrontends() && InspectorInstrumentation::consoleAgentEnabled(state->scriptExecutionContext()))
        maxStackSize = ScriptCallStack::maxCallStackSizeToCapture;

    return RBScriptCallStackFactory::createScriptCallStack(state, maxStackSize);
}

PassRefPtr<ScriptArguments> RBScriptCallStackFactory::createScriptArguments(RBScriptState* state, int argc, VALUE* argv, unsigned skipArgumentCount)
{
    Vector<ScriptValue> arguments;
    for (int i = skipArgumentCount; i < argc; ++i)
        arguments.append(RBScriptValue::scriptValue(argv[i]));
    return ScriptArguments::create(state, arguments);
}

} // namespace WebCore
