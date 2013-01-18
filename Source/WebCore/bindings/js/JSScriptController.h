/*
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 *  Copyright (C) 2013 Eric Seidel (tim.mahoney@me.com)
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

#ifndef JSScriptController_h
#define JSScriptController_h

#include "FrameLoaderTypes.h"
#include "JSDOMWindowShell.h"
#include "ScriptController.h"
#include "ScriptInstance.h"
#include <JavaScriptCore/JSBase.h>
#include <heap/Strong.h>
#include <wtf/Forward.h>
#include <wtf/RefPtr.h>
#include <wtf/text/TextPosition.h>

#if PLATFORM(MAC)
#include <wtf/RetainPtr.h>
OBJC_CLASS WebScriptObject;
OBJC_CLASS JSContext;
#endif

struct NPObject;

namespace JSC {
    class JSGlobalObject;
    class ExecState;

    namespace Bindings {
        class RootObject;
    }
}

namespace WebCore {

class HTMLPlugInElement;
class Frame;
class ScriptSourceCode;
class ScriptValue;
class SecurityOrigin;
class Widget;

typedef HashMap<void*, RefPtr<JSC::Bindings::RootObject> > RootObjectMap;

class JSScriptController : public ScriptController {
    friend class ScriptCachedFrameData;
    typedef WTF::HashMap< RefPtr<DOMWrapperWorld>, JSC::Strong<JSDOMWindowShell> > ShellMap;

public:
    JSScriptController(Frame*);
    virtual ~JSScriptController();

    static PassRefPtr<DOMWrapperWorld> createWorld();

    JSDOMWindowShell* createWindowShell(DOMWrapperWorld*);
    void destroyWindowShell(DOMWrapperWorld*);

    JSDOMWindowShell* windowShell(DOMWrapperWorld* world)
    {
        ShellMap::iterator iter = m_windowShells.find(world);
        return (iter != m_windowShells.end()) ? iter->value.get() : initScript(world);
    }
    JSDOMWindowShell* existingWindowShell(DOMWrapperWorld* world) const
    {
        ShellMap::const_iterator iter = m_windowShells.find(world);
        return (iter != m_windowShells.end()) ? iter->value.get() : 0;
    }
    JSDOMWindow* globalObject(DOMWrapperWorld* world)
    {
        return windowShell(world)->window();
    }

    static void getAllWorlds(Vector<RefPtr<DOMWrapperWorld> >&);

    // Returns true if argument is a JavaScript URL.
    virtual bool executeIfJavaScriptURL(const KURL&, ShouldReplaceDocumentIfJavaScriptURL shouldReplaceDocumentIfJavaScriptURL = ReplaceDocumentIfJavaScriptURL);

    virtual ScriptValue evaluate(const ScriptSourceCode&);
    virtual ScriptValue evaluateInWorld(const ScriptSourceCode&, DOMWrapperWorld*);

    WTF::TextPosition eventHandlerPosition() const;

    void enableEval();
    void disableEval(const String& errorMessage);

    // Debugger can be 0 to detach any existing Debugger.
    virtual void attachDebugger(ScriptDebugServer*); // Attaches/detaches in all worlds/window shells.
    void attachDebugger(JSDOMWindowShell*, ScriptDebugServer*);

    void clearWindowShell(DOMWindow* newDOMWindow, bool goingIntoPageCache);
    void updateDocument();

    void namedItemAdded(HTMLDocument*, const AtomicString&) { }
    void namedItemRemoved(HTMLDocument*, const AtomicString&) { }

    // Notifies the JSScriptController that the securityOrigin of the current
    // document was modified.  For example, this method is called when
    // document.domain is set.  This method is *not* called when a new document
    // is attached to a frame because updateDocument() is called instead.
    void updateSecurityOrigin();

    void clearScriptObjects();
    void cleanupScriptObjectsForPlugin(void*);

    void updatePlatformScriptObjects();

    PassScriptInstance createScriptInstanceForWidget(Widget*);
    JSC::Bindings::RootObject* bindingRootObject();
    JSC::Bindings::RootObject* cacheableBindingRootObject();

    PassRefPtr<JSC::Bindings::RootObject> createRootObject(void* nativeHandle);

#if ENABLE(INSPECTOR)
    static void setCaptureCallStackForUncaughtExceptions(bool);
    void collectIsolatedContexts(Vector<std::pair<ScriptState*, SecurityOrigin*> >&);
#endif

#if PLATFORM(MAC)
    WebScriptObject* windowScriptObject();
    JSContext *javaScriptContext();
#endif

    JSC::JSObject* jsObjectForPluginElement(HTMLPlugInElement*);
    
#if ENABLE(NETSCAPE_PLUGIN_API)
    NPObject* createScriptObjectForPluginElement(HTMLPlugInElement*);
    NPObject* windowScriptNPObject();
#endif

    // FIXME: Stub for parity with V8 implementation. http://webkit.org/b/100815
    bool shouldBypassMainWorldContentSecurityPolicy() { return false; }

private:
    JSDOMWindowShell* initScript(DOMWrapperWorld* world);

    void disconnectPlatformScriptObjects();

    ShellMap m_windowShells;

    // The root object used for objects bound outside the context of a plugin, such
    // as NPAPI plugins. The plugins using these objects prevent a page from being cached so they
    // are safe to invalidate() when WebKit navigates away from the page that contains them.
    RefPtr<JSC::Bindings::RootObject> m_bindingRootObject;
    // Unlike m_bindingRootObject these objects are used in pages that are cached, so they are not invalidate()'d.
    // This ensures they are still available when the page is restored.
    RefPtr<JSC::Bindings::RootObject> m_cacheableBindingRootObject;
    RootObjectMap m_rootObjects;
#if ENABLE(NETSCAPE_PLUGIN_API)
    NPObject* m_windowScriptNPObject;
#endif
#if PLATFORM(MAC)
    RetainPtr<WebScriptObject> m_windowScriptObject;
#endif
};

} // namespace WebCore

#endif // JSScriptController_h
