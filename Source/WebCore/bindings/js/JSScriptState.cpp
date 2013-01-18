/*
 * Copyright (C) 2009, 2011 Google Inc. All rights reserved.
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

#include "config.h"
#include "JSScriptState.h"

#include "Frame.h"
#include "JSDOMWindowBase.h"
#include "JSScriptCallStackFactory.h"
#include "JSScriptController.h"
#include "Node.h"
#include "Page.h"
#include "ScriptArguments.h"
#include "WorkerContext.h"
#include "WorkerScriptController.h"
#include <heap/StrongInlines.h>
#include <interpreter/CallFrame.h>
#include <runtime/JSGlobalObject.h>

#if ENABLE(WORKERS)
#include "JSWorkerContext.h"
#endif

namespace WebCore {

JSScriptState::JSScriptState(JSC::ExecState* execState)
: ScriptState(JSScriptType)
, m_execState(execState)
{
}

JSScriptState* JSScriptState::forExecState(JSC::ExecState* exec)
{
    // FIXME: This probably leaks. What do we do about creating a ScriptState for an ExecState?
    //        Can we cache them somwhere?
    return new JSScriptState(exec);
}
    
bool JSScriptState::hadException()
{
    return m_execState->hadException();
}

DOMWindow* JSScriptState::domWindow() const
{
    JSC::JSGlobalObject* globalObject = execState()->lexicalGlobalObject();
    if (!globalObject->inherits(&JSDOMWindowBase::s_info))
        return 0;
    return JSC::jsCast<JSDOMWindowBase*>(globalObject)->impl();
}

ScriptExecutionContext* JSScriptState::scriptExecutionContext() const
{
    JSC::JSGlobalObject* globalObject = execState()->lexicalGlobalObject();
    if (!globalObject->inherits(&JSDOMGlobalObject::s_info))
        return 0;
    return JSC::jsCast<JSDOMGlobalObject*>(globalObject)->scriptExecutionContext();
}

bool JSScriptState::evalEnabled() const
{
    JSC::JSGlobalObject* globalObject = execState()->lexicalGlobalObject();
    return globalObject->evalEnabled();
}

void JSScriptState::setEvalEnabled(bool enabled)
{
    JSC::JSGlobalObject* globalObject = execState()->lexicalGlobalObject();
    return globalObject->setEvalEnabled(enabled);
}
    
JSScriptState* JSScriptState::mainWorldScriptState(Frame* frame)
{
    if (!frame)
        return 0;
    JSDOMWindowShell* shell = static_cast<JSScriptController*>(frame->script(JSScriptType))->windowShell(mainThreadNormalWorld());
    return JSScriptState::forExecState(shell->window()->globalExec());
}

ScriptState* scriptStateFromNode(DOMWrapperWorld* world, Node* node)
{
    // FIXME: Make generic.
    if (!node)
        return 0;
    Document* document = node->document();
    if (!document)
        return 0;
    Frame* frame = document->frame();
    if (!frame)
        return 0;
    if (!frame->script()->canExecuteScripts(NotAboutToExecuteScript))
        return 0;
    
    JSScriptController* controller = static_cast<JSScriptController*>(frame->script(JSScriptType));
    return JSScriptState::forExecState(controller->globalObject(world)->globalExec());
}

ScriptState* scriptStateFromPage(DOMWrapperWorld* world, Page* page)
{
    // FIXME: Make generic.
    JSScriptController* controller = static_cast<JSScriptController*>(page->mainFrame()->script(JSScriptType));
    return JSScriptState::forExecState(controller->globalObject(world)->globalExec());
}

#if ENABLE(WORKERS)
ScriptState* scriptStateFromWorkerContext(WorkerContext* workerContext)
{
    // FIXME: Make generic.
    return JSScriptState::forExecState(workerContext->script()->workerContextWrapper()->globalExec());
}
#endif

PassRefPtr<ScriptCallStack> JSScriptState::createScriptCallStack(size_t maxStackSize, bool emptyStackIsAllowed)
{
    return JSScriptCallStackFactory::createScriptCallStack(maxStackSize, emptyStackIsAllowed);
}

PassRefPtr<ScriptCallStack> JSScriptState::createScriptCallStack(size_t maxStackSize)
{
    return JSScriptCallStackFactory::createScriptCallStack(execState(), maxStackSize);
}

PassRefPtr<ScriptCallStack> JSScriptState::createScriptCallStackForConsole()
{
    return JSScriptCallStackFactory::createScriptCallStackForConsole(execState());
}

} // namespace WebCore
