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
#include "RBEvent.h"

#include "Event.h"
#include "RBAudioProcessingEvent.h"
#include "RBBeforeLoadEvent.h"
#include "RBClipboard.h"
#include "RBCloseEvent.h"
#include "RBCompositionEvent.h"
#include "RBCustomEvent.h"
#include "RBErrorEvent.h"
#include "RBFocusEvent.h"
#include "RBHashChangeEvent.h"
#include "RBKeyboardEvent.h"
#include "RBMessageEvent.h"
#include "RBMouseEvent.h"
#include "RBMutationEvent.h"
#include "RBOfflineAudioCompletionEvent.h"
#include "RBOverflowEvent.h"
#include "RBPageTransitionEvent.h"
#include "RBPopStateEvent.h"
#include "RBProgressEvent.h"
#include "RBSpeechSynthesisEvent.h"
#include "RBStorageEvent.h"
#include "RBSVGZoomEvent.h"
#include "RBTextEvent.h"
#include "RBTrackEvent.h"
#include "RBTransitionEvent.h"
#include "RBUIEvent.h"
#include "RBWebGLContextEvent.h"
#include "RBWebKitAnimationEvent.h"
#include "RBWebKitTransitionEvent.h"
#include "RBWheelEvent.h"
#include "RBXMLHttpRequestProgressEvent.h"
#include <wtf/PassRefPtr.h>

namespace WebCore {

#define TRY_TO_WRAP_WITH_INTERFACE(interfaceName) \
    if (eventNames().interfaceFor##interfaceName == desiredInterface) \
        return toRB(RB##interfaceName::rubyClass(), event);

VALUE toRB(PassRefPtr<Event> event)
{
    if (!event || !event.get())
        return Qnil;

    String desiredInterface = event.get()->interfaceName();
    DOM_EVENT_INTERFACES_FOR_EACH(TRY_TO_WRAP_WITH_INTERFACE)

    return toRB(RBEvent::rubyClass(), event);
}

VALUE RBEvent::clipboard_data_getter(VALUE self)
{
    Event* event = impl<Event>(self);
    if (event->isClipboardEvent())
        return toRB(event->clipboardData());
    
    return Qnil;
}

} // namespace WebCore
