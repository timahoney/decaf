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
#include "RBSecurityMeasures.h"

#include "RBConverters.h"

namespace RB {

void ensureRubySecurity()
{
    // This is an ongoing list of classes and methods that are
    // unsafe to run from the browser. We want to disable them all.

    // FIXME: Make sure we catch everything.

    // FIXME: Right now, we undefine global hooked and virtual variables
    // by redefining them to be nothing. However, this means that they cannot
    // be used by the user. For example, we'll redefine $?, but then the user
    // can never use the $? variable. This is kind of alright, but it would 
    // be nicer to just remove the global entirely so it can be reused.

    // FIXME: Should we be using the $SAFE level as well?

    // process.c
    rb_const_remove(rb_cObject, rb_intern("Process"));
    rb_undef_method(rb_mKernel, "exec");
    rb_undef_method(rb_mKernel, "fork");
    rb_undef_method(rb_mKernel, "exit!");
    rb_undef_method(rb_mKernel, "system");
    rb_undef_method(rb_mKernel, "spawn");
    // FIXME: Should we allow sleep?
    // rb_undef_method(rb_mKernel, "sleep");
    rb_undef_method(rb_mKernel, "exit");
    rb_undef_method(rb_mKernel, "abort");
    rb_define_virtual_variable("$?", 0, 0);
    rb_define_virtual_variable("$$", 0, 0);

    // file.c
    rb_const_remove(rb_cObject, rb_intern("File"));
    rb_undef_method(rb_mKernel, "test");

    // io.c
    rb_const_remove(rb_cObject, rb_intern("IO"));
    rb_undef_method(rb_mKernel, "syscall");
    rb_undef_method(rb_mKernel, "open");
    rb_undef_method(rb_mKernel, "printf");
    rb_undef_method(rb_mKernel, "print");
    rb_undef_method(rb_mKernel, "putc");
    // FIXME: Uncomment this when releasing.
    // rb_undef_method(rb_mKernel, "puts");
    rb_undef_method(rb_mKernel, "gets");
    rb_undef_method(rb_mKernel, "readline");
    rb_undef_method(rb_mKernel, "select");
    rb_undef_method(rb_mKernel, "readlines");
    rb_undef_method(rb_mKernel, "`");
    // FIXME: Uncomment this when releasing.
    // rb_undef_method(rb_mKernel, "p");
    rb_undef_method(rb_mKernel, "display");
    rb_const_remove(rb_cObject, rb_intern("ARGF"));
    rb_const_remove(rb_cObject, rb_intern("STDIN"));
    // FIXME: For some reason, if we undefine STDOUT, then we get crashes. Why?
    // rb_const_remove(rb_cObject, rb_intern("STDOUT"));
    rb_const_remove(rb_cObject, rb_intern("STDERR"));
    // FIXME: Should we decomment this?
    // rb_define_virtual_variable("$,", 0, 0);
    rb_define_virtual_variable("$<", 0, 0);
    rb_define_virtual_variable("$/", 0, 0);
    rb_define_virtual_variable("$-0", 0, 0);
    rb_define_virtual_variable("$\\", 0, 0);
    rb_define_virtual_variable("$_", 0, 0);
    rb_define_virtual_variable("$.", 0, 0);
    rb_define_virtual_variable("$FILENAME", 0, 0);
    rb_define_virtual_variable("$-i", 0, 0);
    rb_define_virtual_variable("$*", 0, 0);
    rb_define_virtual_variable("$stdin", 0, 0);
    rb_define_virtual_variable("$stdout", 0, 0);
    rb_define_virtual_variable("$stderr", 0, 0);
    rb_define_virtual_variable("$>", 0, 0);

    // load.c
    rb_define_virtual_variable("$:", 0, 0);
    rb_define_virtual_variable("$-I", 0, 0);
    rb_define_virtual_variable("$LOAD_PATH", 0, 0);
    rb_define_virtual_variable("$\"", 0, 0);
    rb_define_virtual_variable("$LOADED_FEATURES", 0, 0);
    rb_undef_method(rb_mKernel, "load");
    rb_undef_method(rb_mKernel, "require");
    rb_undef_method(rb_mKernel, "require_relative");
    rb_undef_method(rb_cModule, "autoload");
    rb_undef_method(rb_cModule, "autoload?");
    rb_undef_method(rb_mKernel, "autoload");
    rb_undef_method(rb_mKernel, "autoload?");

    // ruby.c
    rb_define_virtual_variable("$VERBOSE", 0, 0);
    rb_define_virtual_variable("$-v", 0, 0);
    rb_define_virtual_variable("$-w", 0, 0);
    rb_define_virtual_variable("$-W", 0, 0);
    rb_define_virtual_variable("$DEBUG", 0, 0);
    rb_define_virtual_variable("$-d", 0, 0);
    rb_define_virtual_variable("$0", 0, 0);
    rb_define_virtual_variable("$PROGRAM_NAME", 0, 0);
    rb_const_remove(rb_cObject, rb_intern("ARGV"));
    // rb_const_remove(rb_cObject, rb_intern("DATA"));
    rb_define_virtual_variable("$-p", 0, 0);
    rb_define_virtual_variable("$-l", 0, 0);
    rb_define_virtual_variable("$-a", 0, 0);

    // error.c
    rb_undef_method(rb_mKernel, "warn");

    // signal.c
    rb_const_remove(rb_cObject, rb_intern("Signal"));
    rb_undef_method(rb_mKernel, "trap");

    // eval_jump.c
    rb_undef_method(rb_mKernel, "at_exit");

    // vm_trace.c
    rb_undef_method(rb_mKernel, "set_trace_func");
    rb_undef_method(rb_mKernel, "add_trace_func");

    // vm.c
    rb_const_remove(rb_cObject, rb_intern("TOPLEVEL_BINDING"));

    // gc.c
    VALUE rb_mObSpace = rb_const_get(rb_cObject, rb_intern("ObjectSpace"));
    rb_undef_method(rb_mObSpace, "each_object");
    rb_undef_method(rb_singleton_class(rb_mObSpace), "each_object");
}

} // namespace RB
