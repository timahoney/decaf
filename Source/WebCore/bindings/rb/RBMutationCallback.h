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

#ifndef RBMutationCallback_h
#define RBMutationCallback_h

#include "ActiveDOMCallback.h"
#include "MutationCallback.h"
#include "RBCallback.h"
#include "RBScriptState.h"

namespace WebCore {

class RBMutationCallback : public MutationCallback, public RBCallback, public ActiveDOMCallback {
public:
    static PassRefPtr<RBMutationCallback> create(VALUE proc, ScriptExecutionContext* context)
    {
        return adoptRef(new RBMutationCallback(proc, context));
    }

    virtual ~RBMutationCallback();

    virtual void call(const Vector<RefPtr<MutationRecord> >&, MutationObserver*);
    virtual ScriptExecutionContext* scriptExecutionContext() const { return ContextDestructionObserver::scriptExecutionContext(); }

protected:
    RBMutationCallback(VALUE proc, ScriptExecutionContext*);
};

inline VALUE toRB(RBMutationCallback* impl) { return impl->proc(); }
inline VALUE toRB(PassRefPtr<RBMutationCallback> impl) { return impl.get()->proc(); }

} // namespace WebCore

#endif
