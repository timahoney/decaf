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

#include "RBBiquadFilterNode.h"

#include "RBConverters.h"

namespace WebCore {

VALUE RBBiquadFilterNode::type_setter(VALUE self, VALUE newValue)
{
   BiquadFilterNode* imp = impl<BiquadFilterNode>(self);

#if ENABLE(LEGACY_WEB_AUDIO)
   if (IS_RB_NUM(newValue)) {
       uint32_t type = NUM2UINT(newValue);
       if (!imp->setType(type)) {
           rb_raise(rb_eTypeError, "Illegal BiquadFilterNode type");
           return Qnil;
       }
       return newValue;
   }
#endif

   if (IS_RB_STRING(newValue)) {
       String type = StringValueCStr(newValue);
       if (type == "lowpass" || type == "highpass" || type == "bandpass" || type == "lowshelf" || type == "highshelf" || type == "peaking" || type == "notch" || type == "allpass") {
           imp->setType(type);
           return newValue;
       }
   }
   
   rb_raise(rb_eTypeError, "Illegal BiquadFilterNode type");
   return Qnil;
}

} // namespace WebCore

#endif // ENABLE(WEB_AUDIO)
