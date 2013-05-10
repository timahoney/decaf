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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"

#if ENABLE(WORKERS)

#include "RBWorkerScriptController.h"

#include "RBCallHelpers.h"
#include "RBDedicatedWorkerContext.h"
#include "RBDOMBinding.h"
#include "RBScriptValue.h"
#if ENABLE(SHARED_WORKERS)
#include "RBSharedWorkerContext.h"
#endif
#include "ScriptSourceCode.h"

using namespace RB;

namespace WebCore {

RBWorkerScriptController::RBWorkerScriptController(WorkerContext* workerContext)
    : WorkerScriptController(workerContext, RBScriptType)
    , m_workerContextWrapper(Qnil)
    , m_isTerminating(false)
{
}

template <typename T>
void derefWrappedWorkerContext(VALUE rbWorkerContext)
{
    RBImplWrapper<T>* wrapper;
    Data_Get_Struct(rbWorkerContext, RBImplWrapper<T>, wrapper);
    wrapper->ptr = 0;
}

RBWorkerScriptController::~RBWorkerScriptController()
{
    if (NIL_P(m_workerContextWrapper))
        return;
    
    if (m_workerContext->isDedicatedWorkerContext())
        derefWrappedWorkerContext<DedicatedWorkerContext>(m_workerContextWrapper);
#if ENABLE(SHARED_WORKERS)
    else if (m_workerContext->isSharedWorkerContext())
        derefWrappedWorkerContext<SharedWorkerContext>(m_workerContextWrapper);
#endif
}

VALUE RBWorkerScriptController::workerContextWrapper()
{
    initContextWrapperIfNeeded();
    return m_workerContextWrapper;
}

void RBWorkerScriptController::initContextWrapperIfNeeded()
{
    if (!NIL_P(m_workerContextWrapper))
        return;
    
    if (m_workerContext->isDedicatedWorkerContext())
        m_workerContextWrapper = toRB(static_cast<DedicatedWorkerContext*>(m_workerContext));
#if ENABLE(SHARED_WORKERS)
    else if (m_workerContext->isSharedWorkerContext())
        m_workerContextWrapper = toRB(static_cast<SharedWorkerContext*>(m_workerContext));
#endif
}

void RBWorkerScriptController::evaluate(const ScriptSourceCode& sourceCode)
{
    if (isExecutionForbidden())
        return;

    ScriptValue exception;
    evaluate(sourceCode, &exception);
    if (!exception.hasNoValue()) {
        VALUE exceptionRB = static_cast<RBScriptValue*>(exception.delegate())->rbValue();
        reportException(m_workerContext, exceptionRB);
    }
}

void RBWorkerScriptController::evaluate(const ScriptSourceCode& sourceCode, ScriptValue* exception)
{
    if (isExecutionForbidden())
        return;
    
    if (!m_disableEvalPendingMessage.isEmpty()) {
        // FIXME: Should this do something else?
        String message = m_disableEvalPendingMessage;
        m_disableEvalPendingMessage = String();
        rb_raise(rb_eRuntimeError, "Evaluation disabled: %s", m_disableEvalPendingMessage.utf8().data());
    }

    VALUE workerContextBinding = bindingFromContext(m_workerContext);

    VALUE scriptString = rb_str_new2(sourceCode.source().utf8().data());
    VALUE fileName = rb_str_new2(sourceCode.url().string().utf8().data());
    VALUE lineNumber = INT2NUM(1);
    VALUE evaluationException;
    callFunction(workerContextBinding, "eval", scriptString, fileName, lineNumber, &evaluationException);

    if (!NIL_P(evaluationException) || isExecutionTerminating()) {
        forbidExecution();
        return;
    }

    if (!NIL_P(evaluationException)) {
        String errorMessage;
        int lineNumber = 0;
        String sourceURL = sourceCode.url().string();
        if (m_workerContext->sanitizeScriptError(errorMessage, lineNumber, sourceURL, sourceCode.cachedScript()))
            *exception = RBScriptValue::scriptValue(rb_exc_new2(rb_eRuntimeError, errorMessage.utf8().data()));
        else
            *exception = RBScriptValue::scriptValue(evaluationException);
    }
}

void RBWorkerScriptController::setException(const ScriptValue& exception)
{
    VALUE exceptionRB = static_cast<RBScriptValue*>(exception.delegate())->rbValue();
    rb_exc_raise(exceptionRB);
}

void RBWorkerScriptController::scheduleExecutionTermination()
{
    // The mutex provides a memory barrier to ensure that once
    // termination is scheduled, isExecutionTerminating will
    // accurately reflect that state when called from another thread.
    MutexLocker locker(m_scheduledTerminationMutex);
    m_isTerminating = true;
}

bool RBWorkerScriptController::isExecutionTerminating() const
{
    // See comments in scheduleExecutionTermination regarding mutex usage.
    MutexLocker locker(m_scheduledTerminationMutex);
    return m_isTerminating;
}

void RBWorkerScriptController::disableEval(const String& errorMessage)
{
    m_disableEvalPendingMessage = errorMessage;
}

} // namespace WebCore

#endif // ENABLE(WORKERS)
