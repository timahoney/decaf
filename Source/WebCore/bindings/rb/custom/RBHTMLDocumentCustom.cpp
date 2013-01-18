/*
 *  Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
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

#include "config.h"
#include "RBHTMLDocument.h"

#include "BindingState.h"
#include "HTMLCollection.h"
#include "HTMLIFrameElement.h"
#include "HTMLNames.h"
#include "RBConverters.h"
#include "RBDOMWindow.h"
#include "RBHTMLCollection.h"
#include "RBScriptState.h"
#include <wtf/unicode/CharacterNames.h>

namespace WebCore {

VALUE RBHTMLDocument::named_getter(VALUE self, VALUE name)
{
    HTMLDocument* document = impl<HTMLDocument>(self);
    String propertyName = String(StringValueCStr(name));
    RefPtr<HTMLCollection> collection = document->documentNamedItems(propertyName);
    
    if (collection->isEmpty())
        return Qnil;
    
    if (collection->hasExactlyOneItem()) {
        Node* node = collection->item(0);
        
        Frame* frame;
        if (node->hasTagName(HTMLNames::iframeTag) && (frame = static_cast<HTMLIFrameElement*>(node)->contentFrame()))
            return toRB(frame);
        
        return toRB(node);
    }
    
    return toRB(collection.get());
}

static inline void documentWrite(RBScriptState* state, VALUE text, HTMLDocument* document, bool addNewline)
{
    String string = rbStringOrUndefined(text);
    if (addNewline)
        string.append(String(&newlineCharacter, 1));

    document->write(string, activeDOMWindow(state)->document());
}

VALUE RBHTMLDocument::write(int argc, VALUE* argv, VALUE self)
{
    VALUE text;
    rb_scan_args(argc, argv, "01", &text);

    RBScriptState* state = RBScriptState::current();
    documentWrite(state, text, impl<HTMLDocument>(self), false);
    return Qnil;
}

VALUE RBHTMLDocument::writeln(int argc, VALUE* argv, VALUE self)
{
    VALUE text;
    rb_scan_args(argc, argv, "01", &text);
    
    RBScriptState* state = RBScriptState::current();
    documentWrite(state, text, impl<HTMLDocument>(self), true);
    return Qnil;
}

} // namespace WebCore
