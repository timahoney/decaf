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

#ifndef RBEventListener_h
#define RBEventListener_h

#include "EventListener.h"
#include "RBCallback.h"
#include <Ruby/ruby.h>
#include <WTF/PassRefPtr.h>

namespace WebCore {

class RBEventListener : public EventListener, public RBCallback {
public:
    static PassRefPtr<RBEventListener> create(VALUE proc, bool isAttribute = false)
    {
        return adoptRef(new RBEventListener(proc, isAttribute));
    }

    virtual bool operator==(const EventListener& other);
    virtual void handleEvent(ScriptExecutionContext*, Event*);
    
    static const RBEventListener* cast(const EventListener* listener)
    {
        return listener->type() == RBEventListenerType
            ? static_cast<const RBEventListener*>(listener)
            : 0;
    }
    
protected:
    RBEventListener(VALUE proc, bool isAttribute);
    virtual ~RBEventListener();
    
private:
    bool m_isAttribute;
    
    virtual bool virtualisAttribute() const { return m_isAttribute; }
};

inline VALUE toRB(EventListener* listener)
{
    if (!listener)
        return Qnil;
    
    if (const RBEventListener* rbListener = RBEventListener::cast(listener))
        return rbListener->proc();
    
    return Qnil;
}

} // namespace Webcore

#endif // RBEventListener_h
