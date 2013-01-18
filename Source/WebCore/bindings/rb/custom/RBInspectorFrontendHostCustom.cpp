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

#include "RBInspectorFrontendHost.h"

#include "RBConverters.h"
#include "RBEvent.h"
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

VALUE RBInspectorFrontendHost::platform(VALUE)
{
#if PLATFORM(MAC)
    DEFINE_STATIC_LOCAL(const String, platform, (ASCIILiteral("mac")));
#elif OS(WINDOWS)
    DEFINE_STATIC_LOCAL(const String, platform, (ASCIILiteral("windows")));
#elif OS(LINUX)
    DEFINE_STATIC_LOCAL(const String, platform, (ASCIILiteral("linux")));
#elif OS(FREEBSD)
    DEFINE_STATIC_LOCAL(const String, platform, (ASCIILiteral("freebsd")));
#elif OS(OPENBSD)
    DEFINE_STATIC_LOCAL(const String, platform, (ASCIILiteral("openbsd")));
#elif OS(SOLARIS)
    DEFINE_STATIC_LOCAL(const String, platform, (ASCIILiteral("solaris")));
#else
    DEFINE_STATIC_LOCAL(const String, platform, (ASCIILiteral("unknown")));
#endif
    return rb_str_new2(platform.utf8().data());
}

VALUE RBInspectorFrontendHost::port(VALUE)
{
#if PLATFORM(QT)
    DEFINE_STATIC_LOCAL(const String, port, (ASCIILiteral("qt")));
#elif PLATFORM(GTK)
    DEFINE_STATIC_LOCAL(const String, port, (ASCIILiteral("gtk")));
#elif PLATFORM(WX)
    DEFINE_STATIC_LOCAL(const String, port, (ASCIILiteral("wx")));
#else
    DEFINE_STATIC_LOCAL(const String, port, (ASCIILiteral("unknown")));
#endif
    return rb_str_new2(port.utf8().data());
}

#if ENABLE(CONTEXT_MENUS)
static void populateContextMenuItems(VALUE array, ContextMenu& menu)
{
    for (int i = 0; i < RARRAY_LEN(array); ++i) {
        VALUE item = rb_ary_entry(array, i);
        VALUE label = rb_hash_aref(item, ID2SYM(rb_intern("label")));
        VALUE type = rb_hash_aref(item, ID2SYM(rb_intern("type")));
        VALUE id = rb_hash_aref(item, ID2SYM(rb_intern("id")));
        VALUE enabled = rb_hash_aref(item, ID2SYM(rb_intern("enabled")));
        VALUE checked = rb_hash_aref(item, ID2SYM(rb_intern("checked")));
        VALUE subItems = rb_hash_aref(item, ID2SYM(rb_intern("subItems")));
        if (!IS_RB_STRING(type))
            continue;

        String typeString = StringValueCStr(type);
        if (typeString == "separator") {
            ContextMenuItem item(SeparatorType,
                                 ContextMenuItemCustomTagNoAction,
                                 String());
            menu.appendItem(item);
        } else if (typeString == "subMenu" && IS_RB_ARRAY(subItems)) {
            ContextMenu subMenu;
            populateContextMenuItems(subItems, subMenu);
            ContextMenuItem item(SubmenuType,
                                 ContextMenuItemCustomTagNoAction,
                                 StringValueCStr(label),
                                 &subMenu);
            menu.appendItem(item);
        } else {
            ContextMenuAction typedId = static_cast<ContextMenuAction>(ContextMenuItemBaseCustomTag + NUM2INT(id));
            ContextMenuItem menuItem((typeString == "checkbox" ? CheckableActionType : ActionType), typedId, StringValueCStr(label));
            if (!NIL_P(enabled))
                menuItem.setEnabled(RTEST(enabled));
            if (!NIL_P(checked))
                menuItem.setChecked(RTEST(checked));
            menu.appendItem(menuItem);
        }
    }
}
#endif

VALUE RBInspectorFrontendHost::show_context_menu(VALUE self, VALUE eventRB, VALUE itemsRB)
{
    InspectorFrontendHost* selfImpl = impl<InspectorFrontendHost>(self);

#if ENABLE(CONTEXT_MENUS)
    if (NIL_P(itemsRB) && NIL_P(itemsRB))
        return Qnil;
    Event* event = impl<Event>(eventRB);

    ContextMenu menu;
    populateContextMenuItems(itemsRB, menu);

#if !USE(CROSS_PLATFORM_CONTEXT_MENUS)
    Vector<ContextMenuItem> items = contextMenuItemVector(menu.platformDescription());
#else
    Vector<ContextMenuItem> items = menu.items();
#endif
    selfImpl->showContextMenu(event, items);
#else
    UNUSED_PARAM(eventRB);
    UNUSED_PARAM(itemsRB);
#endif
    return Qnil;
}

VALUE RBInspectorFrontendHost::record_action_taken(VALUE, VALUE)
{
    // FIXME: The JS bindings don't do anything here. Should we?
    return Qnil;
}

VALUE RBInspectorFrontendHost::record_panel_shown(VALUE, VALUE)
{
    // FIXME: The JS bindings don't do anything here. Should we?
    return Qnil;
}

VALUE RBInspectorFrontendHost::record_setting_changed(VALUE, VALUE)
{
    // FIXME: The JS bindings don't do anything here. Should we?
    return Qnil;
}


#endif // ENABLE(INSPECTOR)

} // namespace WebCore
