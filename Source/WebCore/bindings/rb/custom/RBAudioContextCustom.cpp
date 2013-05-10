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

#if ENABLE(WEB_AUDIO)

#include "RBAudioContext.h"

#include "Document.h"
#include "RBExceptionHandler.h"
#include "RBScriptState.h"
#include "RBOfflineAudioContext.h"
#include "ScriptExecutionContext.h"

namespace WebCore {

VALUE RBAudioContext::rb_new(int argc, VALUE* argv, VALUE)
{
    RBScriptState* state = RBScriptState::current();
    ScriptExecutionContext* scriptExecutionContext = state->scriptExecutionContext();
    if (!scriptExecutionContext) {
        rb_raise(rb_eRuntimeError, "AudioContext constructor script execution context is unavailable");
        return Qnil;
    }
    
    if (!scriptExecutionContext->isDocument()) {
        rb_raise(rb_eRuntimeError, "AudioContext constructor called in a script execution context which is not a document");
        return Qnil;
    }
    Document* document = static_cast<Document*>(scriptExecutionContext);

    RefPtr<AudioContext> audioContext;
    
    if (argc == 0) {
        // Constructor for default AudioContext which talks to audio hardware.
        ExceptionCode ec = 0;
        audioContext = AudioContext::create(document, ec);
        if (ec) {
            RB::setDOMException(ec);
            return Qnil;
        }
        if (!audioContext.get()) {
            rb_raise(rb_eRuntimeError, "audio resources unavailable for AudioContext construction");
            return Qnil;
        }
    } else {
        // Constructor for offline (render-target) AudioContext which renders into an AudioBuffer.
        // new AudioContext(in unsigned long numberOfChannels, in unsigned long numberOfFrames, in float sampleRate);
        VALUE numberOfChannelsRB, numberOfFramesRB, sampleRateRB;
        rb_scan_args(argc, argv, "30", &numberOfChannelsRB, &numberOfFramesRB, &sampleRateRB);

        int32_t numberOfChannels = NUM2INT(numberOfChannelsRB);
        int32_t numberOfFrames = NUM2INT(numberOfFramesRB);
        float sampleRate = NUM2DBL(sampleRateRB);
        
        if (numberOfChannels <= 0 || numberOfChannels > 10) {
            rb_raise(rb_eArgError, "Invalid number of channels");
            return Qnil;
        }   

        if (numberOfFrames <= 0) {
            rb_raise(rb_eArgError, "Invalid number of frames");
            return Qnil;
        }

        if (sampleRate <= 0) {
            rb_raise(rb_eArgError, "Invalid sample rate");
            return Qnil;
        }

        ExceptionCode ec = 0;
        audioContext = OfflineAudioContext::create(document, numberOfChannels, numberOfFrames, sampleRate, ec);
        if (ec) {
            RB::setDOMException(ec);
            rb_raise(rb_eRuntimeError, "Error creating OfflineAudioContext");
            return Qnil;
        }
    }

    if (!audioContext.get()) {
        rb_raise(rb_eRuntimeError, "Error creating AudioContext");
        return Qnil;
    }

    return toRB(audioContext.release());
}
    
} // namespace WebCore

#endif // ENABLE(WEB_AUDIO)
