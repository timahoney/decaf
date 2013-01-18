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

#if ENABLE(INSPECTOR)

#include "InjectedScriptManager.h"

#include "JSInjectedScriptManager.h"
#include "RBInjectedScriptManager.h"
#include "BindingSecurity.h"
#include "ExceptionCode.h"
#include "JSDOMWindow.h"
#include "JSDOMWindowCustom.h"
#include "JSInjectedScriptHost.h"
#include "JSMainThreadExecState.h"
#include "JSScriptState.h"
#include "ScriptObject.h"
#include <parser/SourceCode.h>
#include <runtime/JSLock.h>

using namespace JSC;

namespace WebCore {

ScriptObject InjectedScriptManager::createInjectedScript(const String& source, ScriptState* state, int id)
{
    switch (state->scriptType()) {
    case JSScriptType:
        return jsCreateInjectedScript(source, state, id, m_injectedScriptHost.get());
    case RBScriptType:
        return rbCreateInjectedScript(source, state, id, m_injectedScriptHost.get());
    }
}

bool InjectedScriptManager::canAccessInspectedWindow(ScriptState* state)
{
    switch (state->scriptType()) {
    case JSScriptType:
        return jsCanAccessInspectedWindow(state);
    case RBScriptType:
        return rbCanAccessInspectedWindow(state);
    }
}

} // namespace WebCore

#endif // ENABLE(INSPECTOR)
