#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.54)
AC_REVISION([$NiH: configure.ac,v 1.7 2003/10/15 12:16:14 wiz Exp $])
AC_INIT([NeoPop-SDL],[0.1], [tk@giga.or.at], [NeoPop-SDL])
AC_CONFIG_SRCDIR([System_SDL/system_main.c])
AC_CONFIG_HEADERS([System_SDL/config.h])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([dist-bzip2])

# Checks for programs.
AC_PROG_CC

# Checks for typedefs, structures, and compiler characteristics.
AC_STRUCT_TM
AC_C_BIGENDIAN(CFLAGS="${CFLAGS} -DMSB_FIRST", CFLAGS="${CFLAGS} -DLSB_FIRST")

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit gettimeofday memset strchr strdup strerror strrchr strspn])

# Checks for libraries.
AM_PATH_SDL(,,AC_MSG_ERROR([SDL not found]))

AC_ARG_WITH(png,
    AC_HELP_STRING([--with-png=PREFIX],[specify prefix for PNG library]),,
    with_png=yes)

if test "$with_png" != "yes"
then
    CPPFLAGS="$CPPFLAGS -I$with_png/include"
    LDFLAGS="$LDFLAGS -L$with_png/lib -Wl,-R$with_png/lib"
fi
AC_CHECK_HEADER(png.h,
    AC_CHECK_LIB(png, png_create_read_struct, ,
        [AC_CHECK_LIB(png, png_destroy_read_struct,
	    [AC_DEFINE(HAVE_LIBPNG) LIBS="-lpng -lm $LIBS"],
	    [AC_MSG_WARN([PNG library not found -- using BMP screenshots])],
	    -lm)]),
    [AC_MSG_WARN([PNG library not found -- using BMP screenshots])])


AC_CONFIG_FILES([Makefile System_SDL/Makefile])
AC_OUTPUT
