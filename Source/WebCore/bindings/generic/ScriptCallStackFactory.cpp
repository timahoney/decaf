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
#include "ScriptCallStackFactory.h"

#include "JSScriptCallStackFactory.h"
#include "JSScriptState.h"
#include "RBScriptCallStackFactory.h"
#include "RBScriptState.h"
#include <wtf/PassRefPtr.h>

namespace WebCore {
    
// FIXME: Something needs to be done about the organization of these methods.
// Either remove this file entirely in favor of the methods on ScriptState,
// or make it easier to call these methods on each ScriptType without a switch/case.

    
PassRefPtr<ScriptCallStack> createScriptCallStack(size_t maxStackSize, bool emptyStackIsAllowed)
{
    // FIXME: How do we determine which method to call here?
    return JSScriptState::createScriptCallStack(maxStackSize, emptyStackIsAllowed);
}

PassRefPtr<ScriptCallStack> createScriptCallStack(ScriptState* state, size_t maxStackSize)
{
    return state->createScriptCallStack(maxStackSize);
}

PassRefPtr<ScriptCallStack> createScriptCallStackForConsole(ScriptState* state)
{
    return state->createScriptCallStackForConsole();
}
    
} // namespace WebCore
