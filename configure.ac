#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.54)
AC_REVISION([$NiH: configure.ac,v 1.19 2004/08/07 11:54:06 wiz Exp $])
AC_INIT([NeoPop-SDL],[0.2a], [wiz@danbala.tuwien.ac.at], [NeoPop-SDL])
AC_CONFIG_SRCDIR([System_SDL/system_main.c])
AC_CONFIG_HEADERS([System_SDL/config.h])
AC_CANONICAL_HOST
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

AC_CHECK_LIB([z], [main],
	[AC_DEFINE([HAVE_LIBZ], [1],
		  [Define to 1 if you have the zlib library (-lz)])
	LIBS="$LIBS -lz"],
	[AC_MSG_WARN([zlib not found -- no support for zipped ROMs])])

AC_CHECK_LIB([SDL_net], [SDLNet_TCP_Open],
	[AC_DEFINE([HAVE_LIBSDL_NET], [1],
	           [Define to 1 if you have the `SDL_net' library (-lSDL_net)])
	 SDL_LIBS="$SDL_LIBS -lSDL_net"],
	[AC_MSG_WARN([SDL_net not found -- no comms support])],
	[$SDL_LIBS])

# Check for png library
AC_ARG_WITH(png,
    AC_HELP_STRING([--with-png=PREFIX],[specify prefix for PNG library]),,
    with_png=yes)

if test "$with_png" = "no"
then
   AC_MSG_WARN([PNG support disabled -- using BMP screenshots])
else
    if test "$with_png" != "yes" -a "$with_png" != ""
    then
	XPKG_CONFIG_PATH="$PKG_CONFIG_PATH"
	PKG_CONFIG_PATH="$with_png:$with_png/lib/pkgconfig"
    fi
    PKG_CHECK_MODULES(PNG, libpng12 >= 1.2.0,
	[ # png library found
	CPPFLAGS="$CPPFLAGS $PNG_CFLAGS"
	LDFLAGS="$LDFLAGS $PNG_LDFLAGS" ],
	[ # not found, try manually
	AC_MSG_NOTICE([no pkgconfig file for libpng found, trying to find libpng mnually])
	if test "$with_png" -a "$with_png" != "yes"
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
		    [AC_MSG_WARN([PNG library not found -- using BMP screenshots])])])
    # reset PKG_CONFIG PATH
    if test "$with_png" -a "$with_png" != "yes"
    then
	PKG_CONFIG_PATH="$XPKG_CONFIG_PATH"
    fi
fi   

# Man pages
case $host_os
in
	*bsd*) MANFMT=mdoc;;
	*) MANFMT=man;;
esac

AC_SUBST(MANFMT, [${MANFMT}])
AC_SUBST(DOCDIR, [${datadir}/doc/${PACKAGE}])

AC_CONFIG_FILES([Makefile Documentation/Makefile System_SDL/Makefile])
AC_OUTPUT
