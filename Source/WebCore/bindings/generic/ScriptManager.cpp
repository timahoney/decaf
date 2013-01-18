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
#include "ScriptManager.h"

#include "RBScriptController.h"
#include "ScriptController.h"
#include "ScriptType.h"
#include <wtf/HashIterators.h>
#include <wtf/HashMap.h>
#include <wtf/HashTable.h>
#include <wtf/text/CString.h>
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

ScriptManager::ScriptManager(Frame* frame)
: m_frame(frame)
, m_jsController(frame)
, m_rbController(frame)
{
    m_controllers.set(JSScriptType, &m_jsController);
    m_controllers.set(RBScriptType, &m_rbController);
}

ScriptManager::~ScriptManager()
{
}

ScriptController* ScriptManager::scriptController(ScriptType type)
{
    return m_controllers.get(type);
}

bool ScriptManager::shouldBypassMainWorldContentSecurityPolicy()
{
#if USE(V8)
    if (DOMWrapperWorld* world = worldForEnteredContextIfIsolated())
        return world->isolatedWorldHasContentSecurityPolicy();
#endif

    return false;
}

bool ScriptManager::canExecuteScripts(ReasonForCallingCanExecuteScripts reason)
{
    // FIXME: Is there a reason why one ScriptController can and the other can't execute scripts?
    return m_jsController.canExecuteScripts(reason);
}

JSDOMWindowShell* ScriptManager::existingWindowShell(DOMWrapperWorld* world) const
{
    // FIXME: Check the RB version once we make a Ruby DOMWrapperWorld.
    return m_jsController.existingWindowShell(world);
}

} // namespace WebCore
