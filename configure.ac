dnl Process this file with autoconf to produce a configure script.

dnl init

AC_PREREQ(2.56)
AC_REVISION([$NiH: configure.ac,v 1.1 2002/10/25 10:39:20 wiz Exp $])
AC_INIT([neopop-SDL],[0.1])
AC_CONFIG_SRCDIR([SDL/system_main.c])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_C_BIGENDIAN(CFLAGS="${CFLAGS} -DMSB_FIRST", CFLAGS="${CFLAGS} -DLSB_FIRST")

dnl libraries

AM_PATH_SDL(,,AC_MSG_ERROR([SDL not found]))

dnl compiler characteristics

AC_CONFIG_FILES([Makefile SDL/Makefile])
AC_OUTPUT
