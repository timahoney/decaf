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

#ifndef RBWorkerScriptController_h
#define RBWorkerScriptController_h

#if ENABLE(WORKERS)
#include "WorkerScriptController.h"
#include <Ruby/ruby.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class RBWorkerScriptController : public WorkerScriptController {
    WTF_MAKE_NONCOPYABLE(RBWorkerScriptController); WTF_MAKE_FAST_ALLOCATED;
public:
    RBWorkerScriptController(WorkerContext*);
    virtual ~RBWorkerScriptController();

    VALUE workerContextWrapper();

    virtual void evaluate(const ScriptSourceCode&);
    virtual void evaluate(const ScriptSourceCode&, ScriptValue* exception);

    virtual void setException(const ScriptValue&);

    virtual void scheduleExecutionTermination();
    virtual bool isExecutionTerminating() const;

    virtual void disableEval(const String& errorMessage);

private:
    void initContextWrapperIfNeeded();

    mutable Mutex m_scheduledTerminationMutex;
    String m_disableEvalPendingMessage;
    VALUE m_workerContextWrapper;
    bool m_isTerminating;
};

} // namespace WebCore

#endif // ENABLE(WORKERS)

#endif // RBWorkerScriptController_h
