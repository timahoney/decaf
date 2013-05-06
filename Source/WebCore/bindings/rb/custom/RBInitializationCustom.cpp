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
#include "RBDOMBinding.h"
#include "RBFileCustom.h"
#include "RBFileListCustom.h"
#include "RBImageDataCustom.h"
#include "RBMessagePortCustom.h"
#include "RBWorker.h"

using namespace RB;

namespace WebCore {

extern "C" VALUE worker_new_custom(int, VALUE*, VALUE);

void RBInitializationCustom::initializeCustomRubyClasses()
{
    RBArrayBufferCustom::Init_ArrayBufferCustom();
    RBBlobCustom::Init_BlobCustom();
    RBDataViewCustom::Init_DataViewCustom();
    RBFileCustom::Init_FileCustom();
    RBFileListCustom::Init_FileListCustom();
    RBImageDataCustom::Init_ImageDataCustom();
    RBMessagePortCustom::Init_MessagePortCustom();
    
    // FIXME: Remove this when Ruby is ready for Workers.
    rb_define_singleton_method(RBWorker::rubyClass(), "new", RUBY_METHOD_FUNC(worker_new_custom), -1);
}

VALUE worker_new_custom(int argc, VALUE* argv, VALUE self)
{
    VALUE scriptName;
    rb_scan_args(argc, argv, "10", &scriptName);
    if (IS_RB_STRING(scriptName)) {
        KURL url = currentContext()->completeURL(rbToString(scriptName));
        ScriptType scriptType = scriptTypeFromUrl(url);
        if (scriptType == RBScriptType) {
            VALUE exception = rb_exc_new2(rb_eNotImpError, "Cannot currently run a Ruby script in a Worker.");
            rb_funcall(exception, rb_intern("set_backtrace"), 1, rb_make_backtrace());
            reportException(currentContext(), exception);
            return Qnil;
        }
    }
    
    return RBWorker::rb_new(argc, argv, self);
}

} // namespace WebCore
