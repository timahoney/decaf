# $Id: extconf.rb 25189 2009-10-02 12:04:37Z akr $

require 'mkmf'
have_func('rb_block_call', 'ruby/ruby.h')
create_makefile 'racc/cparse'
