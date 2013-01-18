# $RoughId: extconf.rb,v 1.6 2001/07/13 15:38:27 knu Exp $
# $Id: extconf.rb 25189 2009-10-02 12:04:37Z akr $

require "mkmf"

$INSTALLFILES = {
  "digest.h" => "$(HDRDIR)"
}

create_makefile("digest")
