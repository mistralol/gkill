AC_INIT(gkill, 1.0.0, james@stev.org)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([src/gkill.c])
AC_CONFIG_MACRO_DIR([m4])

LT_INIT
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_PROG_LIBTOOL

AX_COMPILER_FLAGS([WARN_CFLAGS],[WARN_LDFLAGS])

AC_CHECK_FUNCS([asprintf strndupa],,AC_DEFINE([_GNU_SOURCE], [], [Linux requires _GNU_SOURCE for asprintf]))

AC_OUTPUT(Makefile src/Makefile)


