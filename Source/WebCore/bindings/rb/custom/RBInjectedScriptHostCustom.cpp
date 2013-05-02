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

#include "RBInjectedScriptHost.h"

#include "InspectorDOMAgent.h"
#include "RBConverters.h"
#include "RBDOMBinding.h"
#include "RBDOMWindow.h"
#include "RBEventListener.h"
#include "RBHTMLAllCollection.h"
#include "RBHTMLCollection.h"
#include "RBNode.h"
#include "RBNodeList.h"
#include "RBScriptState.h"
#include "RBScriptValue.h"

using namespace RB;

namespace WebCore {

VALUE RBInjectedScriptHost::inspect(VALUE self, VALUE objectId, VALUE rbHints)
{
    if (!NIL_P(objectId) && !NIL_P(rbHints)) {
        ScriptValue object = RBScriptValue::scriptValue(objectId);
        ScriptValue hints = RBScriptValue::scriptValue(rbHints);
        RBScriptState* state = RBScriptState::current();
        impl<InjectedScriptHost>(self)->inspectImpl(object.toInspectorValue(state), hints.toInspectorValue(state));
    }

    return Qnil;
}

VALUE RBInjectedScriptHost::inspected_object(VALUE self, VALUE num)
{
    if (NIL_P(num))
        return Qnil;
    
    InjectedScriptHost::InspectableObject* object = impl<InjectedScriptHost>(self)->inspectedObject(NUM2INT(num));
    if (!object)
        return Qnil;

    ScriptValue scriptValue = object->get(RBScriptState::current());
    if (scriptValue.hasNoValue())
        return Qnil;

    return static_cast<RBScriptValue*>(scriptValue.delegate())->rbValue();
}

VALUE RBInjectedScriptHost::internal_constructor_name(VALUE, VALUE object)
{
    if (NIL_P(object))
        return Qnil;

    VALUE klass = rb_funcall(object, rb_intern("class"), 0);
    VALUE klassName = rb_funcall(klass, rb_intern("name"), 0);    
    String className = rbToString(klassName);
    return rb_str_new2(className.utf8().data());
}

VALUE RBInjectedScriptHost::is_html_all_collection(VALUE, VALUE object)
{
    if (NIL_P(object))
        return Qnil;

    return IS_RB_KIND(object, HTMLAllCollection);
}

VALUE RBInjectedScriptHost::type(VALUE, VALUE object)
{
    if (NIL_P(object))
        return Qnil;

    if (IS_RB_STRING(object))
        return rb_str_new2("string");
    if (IS_RB_ARRAY(object))
        return rb_str_new2("array");
    if (IS_RB_BOOL(object))
        return rb_str_new2("boolean");
    if (IS_RB_NUM(object))
        return rb_str_new2("number");
    if (IS_RB_HASH(object))
        return Qnil;
    if (TYPE(object) == T_REGEXP)
        return rb_str_new2("regexp");
    if (CLASS_OF(object) == rb_cTime)
        return rb_str_new2("date");
    if (IS_RB_KIND(object, Node))
        return rb_str_new2("node");
    if (IS_RB_KIND(object, NodeList))
        return rb_str_new2("array");
    if (IS_RB_KIND(object, HTMLCollection))
        return rb_str_new2("array");

    return Qnil;
}

VALUE RBInjectedScriptHost::function_details(VALUE, VALUE value)
{
    if (NIL_P(value))
        return Qnil;

    VALUE klass = CLASS_OF(value);
    if (klass != rb_cProc && klass != rb_cMethod)
        return Qnil;

    VALUE sourceLocation = rb_funcall(value, rb_intern("source_location"), 0);
    if (NIL_P(sourceLocation))
        return Qnil;
    VALUE fileName = rb_ary_entry(sourceLocation, 0);
    VALUE lineNumber = rb_ary_entry(sourceLocation, 1);
    intptr_t sourceID = sourceIDFromFileName(rbToString(fileName).utf8().data());
    VALUE location = rb_hash_new();
    rb_hash_aset(location, ID2SYM(rb_intern("lineNumber")), lineNumber);
    rb_hash_aset(location, ID2SYM(rb_intern("scriptId")), toRB(String::number(sourceID)));

    VALUE result = rb_hash_new();
    rb_hash_aset(result, ID2SYM(rb_intern("location")), location);
    rb_hash_aset(result, ID2SYM(rb_intern("name")), fileName);

    // FIXME: Provide function scope data in "scopesRaw" property.
    //     https://bugs.webkit.org/show_bug.cgi?id=87192
    return result;
}

VALUE RBInjectedScriptHost::get_internal_properties(VALUE self, VALUE object)
{
    UNUSED_PARAM(self);
    UNUSED_PARAM(object);
    // FIXME: Implement this. At the time of writing, JS didn't implement this either.
    //     https://bugs.webkit.org/show_bug.cgi?id=94533
    return Qnil;
}

static VALUE getRBListenerFunctions(const EventListenerInfo& listenerInfo)
{
    VALUE result = rb_ary_new();
    size_t handlersCount = listenerInfo.eventListenerVector.size();
    for (size_t i = 0; i < handlersCount; ++i) {
        const RBEventListener* rbListener = RBEventListener::cast(listenerInfo.eventListenerVector[i].listener.get());
        if (!rbListener)
            continue;

        // FIXME: Hide listeners from other contexts.
        // if (jsListener->isolatedWorld() != currentWorld(exec))
        //     continue;

        VALUE proc = rbListener->proc();
        if (!proc)
            continue;
        VALUE listenerEntry = rb_hash_new();
        rb_hash_aset(listenerEntry, ID2SYM(rb_intern("listener")), proc);
        rb_hash_aset(listenerEntry, ID2SYM(rb_intern("use_capture")), toRB(listenerInfo.eventListenerVector[i].useCapture));

        rb_ary_push(result, listenerEntry);
    }
    return result;
}

VALUE RBInjectedScriptHost::get_event_listeners(VALUE self, VALUE value)
{
    if (NIL_P(value))
        return Qnil;
    
    Node* node = impl<Node>(value);
    if (!node)
        return Qnil;

    // This can only happen for orphan DocumentType nodes.
    if (!node->document())
        return Qnil;

    Vector<EventListenerInfo> listenersArray;
    impl<InjectedScriptHost>(self)->getEventListenersImpl(node, listenersArray);

    VALUE result = rb_hash_new();
    for (size_t i = 0; i < listenersArray.size(); ++i) {
        VALUE listeners = getRBListenerFunctions(listenersArray[i]);
        if (!RARRAY_LEN(listeners))
            continue;
        AtomicString eventType = listenersArray[i].eventType;
        rb_hash_aset(result, ID2SYM(rb_intern(eventType.string().utf8().data())), listeners);
    }

    return result;
}

VALUE RBInjectedScriptHost::database_id(VALUE self, VALUE rbDatabase)
{
    if (NIL_P(rbDatabase))
        return Qnil;
#if ENABLE(SQL_DATABASE)
    Database* database = impl<Database>(rbDatabase);
    if (database)
        return rb_str_new2(impl<InjectedScriptHost>(self)->databaseIdImpl(database).utf8().data());
#endif
    return Qnil;
}

VALUE RBInjectedScriptHost::storage_id(VALUE self, VALUE rbStorage)
{
    if (NIL_P(rbStorage))
        return Qnil;
    Storage* storage = impl<Storage>(rbStorage);
    if (storage)
        return rb_str_new2(impl<InjectedScriptHost>(self)->storageIdImpl(storage).utf8().data());
    return Qnil;
}

VALUE RBInjectedScriptHost::evaluate(VALUE self, VALUE expression)
{
    if (!IS_RB_STRING(expression)) {
        rb_raise(rb_eArgError, "String argument expected.");
        return Qnil;
    }
    
    if (rbToString(expression) == "this")
        expression = rb_str_new2("self");
    
    // FIXME: Can we get by without using the instance variable here?
    // We might be able to just use currentContext() and bindingFromContext().
    VALUE windowRB = rb_iv_get(self, "@inspected_window");
    DOMWindow* window = impl<DOMWindow>(windowRB);
    VALUE binding = bindingFromContext(window->document());
    
    VALUE result = rb_funcall(binding, rb_intern("eval"), 2, expression, rb_str_new2(""));
    return result;
}

VALUE RBInjectedScriptHost::set_function_variable_value(VALUE, VALUE, VALUE, VALUE, VALUE)
{
    // FIXME: implement this. https://bugs.webkit.org/show_bug.cgi?id=107830
    rb_raise(rb_eTypeError, "Variable value mutation is not supported");
    return Qnil;
}

#endif // ENABLE(INSPECTOR)

} // namespace WebCore
