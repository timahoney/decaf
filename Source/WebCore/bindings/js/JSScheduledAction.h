/*
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reseved.
 *  Copyright (C) 2013 Tim Mahoney (tim.mahoney@me.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef JSScheduledAction_h
#define JSScheduledAction_h

#include "ScheduledAction.h"
#include "JSDOMBinding.h"
#include <heap/Strong.h>
#include <heap/StrongInlines.h>
#include <runtime/JSCell.h>

namespace JSC {
    class JSGlobalObject;
}

namespace WebCore {

class Document;
class ContentSecurityPolicy;
class ScriptExecutionContext;
class WorkerContext;

class JSScheduledAction : public ScheduledAction {
public:
    static PassOwnPtr<JSScheduledAction> create(JSC::ExecState*, DOMWrapperWorld* isolatedWorld, ContentSecurityPolicy*);
    
    virtual ~JSScheduledAction();
    
protected:
    JSScheduledAction(const String& code, DOMWrapperWorld* isolatedWorld)
    : m_function(*isolatedWorld->globalData())
    , m_code(code)
    , m_isolatedWorld(isolatedWorld)
    {
    }
    
    virtual void execute(Document*);
#if ENABLE(WORKERS)
    virtual void execute(WorkerContext*);
#endif

private:
    JSScheduledAction(JSC::ExecState*, JSC::JSValue function, DOMWrapperWorld* isolatedWorld);
    
    void executeFunctionInContext(JSC::JSGlobalObject*, JSC::JSValue thisValue, ScriptExecutionContext*);

    JSC::Strong<JSC::Unknown> m_function;
    Vector<JSC::Strong<JSC::Unknown> > m_args;
    String m_code;
    RefPtr<DOMWrapperWorld> m_isolatedWorld;
};

} // namespace WebCore

#endif // ScheduledAction_h
