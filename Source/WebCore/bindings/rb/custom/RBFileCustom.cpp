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
#include "RBFile.h"

#include "File.h"
#include "RBConverters.h"
#include "RBFileCustom.h"

namespace WebCore {

VALUE RBFileCustom::marshal_load(VALUE, VALUE data)
{
    VALUE rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));
    VALUE values = rb_funcall(rb_mMarshal, rb_intern("load"), 1, data);
    
    String path = rbToString(rb_ary_entry(values, 0));
    KURL url = KURL(KURL(), rbToString(rb_ary_entry(values, 1)));
    String type = rbToString(rb_ary_entry(values, 2));
    RefPtr<File> file = File::create(path, url, type);
    return toRB(file.release());
}

VALUE RBFileCustom::marshal_dump(VALUE self, VALUE)
{
    File* file = impl<File>(self);
    VALUE array = rb_ary_new2(3);
    rb_ary_push(array, toRB(file->path()));
    rb_ary_push(array, toRB(file->url()));
    rb_ary_push(array, toRB(file->type()));
    
    VALUE rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));
    VALUE data = rb_funcall(rb_mMarshal, rb_intern("dump"), 1, array);
    return data;
}

void RBFileCustom::Init_FileCustom()
{
    rb_define_method(RBFile::rubyClass(), "_dump", RUBY_METHOD_FUNC(&RBFileCustom::marshal_dump), 1);
    rb_define_module_function(RBFile::rubyClass(), "_load", RUBY_METHOD_FUNC(&RBFileCustom::marshal_load), 1);
}
    
} // namespace WebCore
