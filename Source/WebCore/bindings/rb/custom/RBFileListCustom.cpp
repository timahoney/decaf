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
#include "RBFileList.h"

#include "FileList.h"
#include "RBConverters.h"
#include "RBFile.h"
#include "RBFileListCustom.h"

namespace WebCore {

VALUE RBFileListCustom::marshal_load(VALUE, VALUE data)
{
    VALUE rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));
    VALUE values = rb_funcall(rb_mMarshal, rb_intern("load"), 1, data);

    unsigned length = NUM2UINT(rb_ary_entry(values, 0));
    VALUE filesRB = rb_ary_entry(values, 1);
    RefPtr<FileList> result = FileList::create();
    for (unsigned i = 0; i < length; i++) {
        VALUE fileRB = rb_ary_entry(filesRB, i);
        RefPtr<File> file = impl<File>(fileRB);
        result->append(file.get());
    }
    
    return toRB(result.release());
}

VALUE RBFileListCustom::marshal_dump(VALUE self, VALUE)
{
    FileList* list = impl<FileList>(self);
    VALUE dataArray = rb_ary_new2(2);
    unsigned length = list->length();
    rb_ary_push(dataArray, toRB(length));

    VALUE filesArray = rb_ary_new2(length);
    for (unsigned i = 0; i < length; i++)
        rb_ary_push(filesArray, toRB(list->item(i)));
    rb_ary_push(dataArray, filesArray);
    
    VALUE rb_mMarshal = rb_const_get(rb_cObject, rb_intern("Marshal"));
    VALUE data = rb_funcall(rb_mMarshal, rb_intern("dump"), 1, dataArray);
    return data;
}

void RBFileListCustom::Init_FileListCustom()
{
    rb_define_method(RBFileList::rubyClass(), "_dump", RUBY_METHOD_FUNC(&RBFileListCustom::marshal_dump), 1);
    rb_define_module_function(RBFileList::rubyClass(), "_load", RUBY_METHOD_FUNC(&RBFileListCustom::marshal_load), 1);
}
    
} // namespace WebCore
