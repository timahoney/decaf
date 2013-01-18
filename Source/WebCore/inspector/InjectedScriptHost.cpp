/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Matt Lilek <webkit@mattlilek.com>
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(INSPECTOR)

#include "InjectedScriptHost.h"

#include "Element.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "HTMLFrameOwnerElement.h"
#include "InjectedScript.h"
#include "InspectorAgent.h"
#include "InspectorClient.h"
#include "InspectorConsoleAgent.h"
#include "InspectorDOMAgent.h"
#include "InspectorDOMStorageAgent.h"
#include "InspectorDatabaseAgent.h"
#include "InspectorDebuggerAgent.h"
#include "InspectorFrontend.h"
#include "InspectorValues.h"
#include "JSNode.h"
#include "JSScriptState.h"
#include "Pasteboard.h"
#include "RBNode.h"
#include "RBScriptValue.h"
#include "Storage.h"

#if ENABLE(SQL_DATABASE)
#include "Database.h"
#endif

#include "markup.h"

#include <wtf/RefPtr.h>
#include <wtf/StdLibExtras.h>

using namespace std;

namespace WebCore {

PassRefPtr<InjectedScriptHost> InjectedScriptHost::create()
{
    return adoptRef(new InjectedScriptHost());
}

InjectedScriptHost::InjectedScriptHost()
    : m_inspectorAgent(0)
    , m_consoleAgent(0)
#if ENABLE(SQL_DATABASE)
    , m_databaseAgent(0)
#endif
    , m_domStorageAgent(0)
    , m_domAgent(0)
{
    m_defaultInspectableObject = adoptPtr(new InspectableObject());
}

InjectedScriptHost::~InjectedScriptHost()
{
}

void InjectedScriptHost::disconnect()
{
    m_inspectorAgent = 0;
    m_consoleAgent = 0;
#if ENABLE(SQL_DATABASE)
    m_databaseAgent = 0;
#endif
    m_domStorageAgent = 0;
    m_domAgent = 0;
}

void InjectedScriptHost::inspectImpl(PassRefPtr<InspectorValue> object, PassRefPtr<InspectorValue> hints)
{
    if (m_inspectorAgent) {
        RefPtr<TypeBuilder::Runtime::RemoteObject> remoteObject = TypeBuilder::Runtime::RemoteObject::runtimeCast(object);
        m_inspectorAgent->inspect(remoteObject, hints->asObject());
    }
}

void InjectedScriptHost::getEventListenersImpl(Node* node, Vector<EventListenerInfo>& listenersArray)
{
    if (m_domAgent)
        m_domAgent->getEventListeners(node, listenersArray, false);
}

void InjectedScriptHost::clearConsoleMessages()
{
    if (m_consoleAgent) {
        ErrorString error;
        m_consoleAgent->clearMessages(&error);
    }
}

void InjectedScriptHost::copyText(const String& text)
{
    Pasteboard::generalPasteboard()->writePlainText(text, Pasteboard::CannotSmartReplace);
}

ScriptValue InjectedScriptHost::InspectableObject::get(ScriptState*)
{
    return ScriptValue();
};

void InjectedScriptHost::addInspectedObject(PassOwnPtr<InjectedScriptHost::InspectableObject> object)
{
    m_inspectedObjects.prepend(object);
    while (m_inspectedObjects.size() > 5)
        m_inspectedObjects.removeLast();
}

void InjectedScriptHost::clearInspectedObjects()
{
    m_inspectedObjects.clear();
}

InjectedScriptHost::InspectableObject* InjectedScriptHost::inspectedObject(unsigned int num)
{
    if (num >= m_inspectedObjects.size())
        return m_defaultInspectableObject.get();
    return m_inspectedObjects[num].get();
}

#if ENABLE(SQL_DATABASE)
String InjectedScriptHost::databaseIdImpl(Database* database)
{
    if (m_databaseAgent)
        return m_databaseAgent->databaseId(database);
    return String();
}
#endif

String InjectedScriptHost::storageIdImpl(Storage* storage)
{
    if (m_domStorageAgent)
        return m_domStorageAgent->storageId(storage);
    return String();
}

#if ENABLE(JAVASCRIPT_DEBUGGER)
ScriptDebugServer& InjectedScriptHost::scriptDebugServer(ScriptType type)
{
    return m_debuggerAgent->scriptDebugServer(type);
}
#endif

Node* InjectedScriptHost::scriptValueAsNode(ScriptValue value)
{
    switch (value.scriptType()) {
    case JSScriptType:
        if (!value.isObject() || value.isNull())
            return 0;
        return toNode(value.jsValue());
        
    case RBScriptType:
        return impl<Node>(static_cast<RBScriptValue*>(value.delegate())->rbValue());
    }

}

ScriptValue InjectedScriptHost::nodeAsScriptValue(ScriptState* scriptState, Node* node)
{
    switch (scriptState->scriptType()) {
    case JSScriptType:
    {
        JSC::ExecState* state = static_cast<JSScriptState*>(scriptState)->execState();
        
        if (!shouldAllowAccessToNode(state, node))
            return ScriptValue(state->globalData(), JSC::jsNull());
        
        JSC::JSLockHolder lock(state);
        return ScriptValue(state->globalData(), toJS(state, deprecatedGlobalObjectForPrototype(state), node));
    }
        
    case RBScriptType:
        return RBScriptValue::scriptValue(toRB(node));
    }
}

} // namespace WebCore

#endif // ENABLE(INSPECTOR)
