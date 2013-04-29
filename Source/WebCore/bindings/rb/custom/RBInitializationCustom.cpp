/*
 * Copyright (C) 2012 Tim Mahoney (tim.mahoney@me.com)
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
#include "RBInitializationCustom.h"

#include "RBArrayBufferCustom.h"
#include "RBBlobCustom.h"
#include "RBConverters.h"
#include "RBDataViewCustom.h"
#include "RBFileCustom.h"
#include "RBFileListCustom.h"
#include "RBImageDataCustom.h"
#include "RBMessagePortCustom.h"
#include "RBWorker.h"

namespace WebCore {

extern "C" VALUE unimplemented(VALUE);

void RBInitializationCustom::initializeCustomRubyClasses()
{
    RBArrayBufferCustom::Init_ArrayBufferCustom();
    RBBlobCustom::Init_BlobCustom();
    RBDataViewCustom::Init_DataViewCustom();
    RBFileCustom::Init_FileCustom();
    RBFileListCustom::Init_FileListCustom();
    RBImageDataCustom::Init_ImageDataCustom();
    RBMessagePortCustom::Init_MessagePortCustom();
}

VALUE unimplemented(VALUE self)
{
    rb_raise(rb_eNotImpError,
             "%s.%s is not implemented in Decaf at the moment.",
             rbToString(self).utf8().data(),
             rb_id2name(rb_frame_this_func()));
}

} // namespace WebCore
