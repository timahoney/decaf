#define RUBY_VERSION "2.0.0"
#define RUBY_RELEASE_DATE "2013-02-24"
#define RUBY_PATCHLEVEL 0

#define RUBY_RELEASE_YEAR 2013
#define RUBY_RELEASE_MONTH 2
#define RUBY_RELEASE_DAY 24

#include "ruby/version.h"

#if !defined RUBY_LIB_VERSION && defined RUBY_LIB_VERSION_STYLE
# if RUBY_LIB_VERSION_STYLE == 3
#   define RUBY_LIB_VERSION RB_STRINGIZE(RUBY_API_VERSION_MAJOR)"."RB_STRINGIZE(RUBY_API_VERSION_MINOR)"."RB_STRINGIZE(RUBY_API_VERSION_TEENY)
# elif RUBY_LIB_VERSION_STYLE == 2
#   define RUBY_LIB_VERSION RB_STRINGIZE(RUBY_API_VERSION_MAJOR)"."RB_STRINGIZE(RUBY_API_VERSION_MINOR)
# endif
#endif

#if RUBY_PATCHLEVEL == -1
#define RUBY_PATCHLEVEL_STR "dev"
#else
#define RUBY_PATCHLEVEL_STR "p"RB_STRINGIZE(RUBY_PATCHLEVEL)
#endif

#ifndef RUBY_REVISION
# include "revision.h"
#endif
# ifndef RUBY_REVISION
# define RUBY_REVISION 0
#endif

#if RUBY_REVISION
# ifdef RUBY_BRANCH_NAME
#  define RUBY_REVISION_STR " "RUBY_BRANCH_NAME" "RB_STRINGIZE(RUBY_REVISION)
# else
#  define RUBY_REVISION_STR " revision "RB_STRINGIZE(RUBY_REVISION)
# endif
#else
# define RUBY_REVISION_STR ""
#endif

# define RUBY_DESCRIPTION	    \
    "ruby "RUBY_VERSION		    \
    RUBY_PATCHLEVEL_STR		    \
    " ("RUBY_RELEASE_DATE	    \
    RUBY_REVISION_STR") "	    \
    "["RUBY_PLATFORM"]"
# define RUBY_COPYRIGHT		    \
    "ruby - Copyright (C) "	    \
    RB_STRINGIZE(RUBY_BIRTH_YEAR)"-"   \
    RB_STRINGIZE(RUBY_RELEASE_YEAR)" " \
    RUBY_AUTHOR
