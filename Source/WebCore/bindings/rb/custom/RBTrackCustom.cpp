/*
 * Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "RBTrackCustom.h"

#include "RBConverters.h"
#include "RBTextTrack.h"

namespace WebCore {

TrackBase* rbToTrack(VALUE track)
{
    if (NIL_P(track))
        return 0;

    if (IS_RB_KIND(track, TextTrack))
        return impl<TextTrack>(track);
    
    // FIXME: Fill in additional tests and casts here for VideoTrack and AudioTrack when 
    // they have been added to WebCore.
    
    return 0;
}

VALUE toRB(PassRefPtr<TrackBase> track)
{
    if (!track || !track.get())
        return Qnil;
    
    switch (track->type()) {
    case TrackBase::BaseTrack:
        // This should never happen.
        ASSERT_NOT_REACHED();
        break;
        
    case TrackBase::AudioTrack:
    case TrackBase::VideoTrack:
        // This should not happen until VideoTrack and AudioTrack are implemented.
        ASSERT_NOT_REACHED();
        break;

    case TrackBase::TextTrack:
        return toRB(RBTextTrack::rubyClass(), track);
    }
    
    return Qnil;
}

} // namespace WebCore
