#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ble-conn-detector], [1.0], [nemea@cesnet.cz])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_MACRO_DIR([../m4])

AX_LIBTRAP_CHECK
AX_UNIREC_CHECK
AX_NEMEACOMMON_CHECK

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for NEMEA libraries.
TRAPLIB=""
PKG_CHECK_MODULES([libtrap], [libtrap], [TRAPLIB="yes"])
if test -n "$TRAPLIB"; then
  LDFLAGS="$libtrap_LDFLAGS $LDFLAGS"
  LIBS="$libtrap_LIBS $LIBS"
  CFLAGS="$libtrap_CFLAGS $CFLAGS"
  CXXFLAGS="$libtrap_CFLAGS $CXXFLAGS"
else
  AC_MSG_ERROR([Libtrap was not found.])
fi

UNIRECLIB=""
PKG_CHECK_MODULES([unirec], [unirec], [UNIRECLIB="yes"])
if test -n "$UNIRECLIB"; then
  LDFLAGS="$unirec_LDFLAGS $LDFLAGS"
  LIBS="$unirec_LIBS $LIBS"
  CFLAGS="$unirec_CFLAGS $CFLAGS"
  CXXFLAGS="$unirec_CFLAGS $CXXFLAGS"
else
  AC_MSG_ERROR([unirec was not found.])
fi

AC_PATH_PROG(UNIRECPROC, ur_processor.sh, [], [/usr/bin/nemea/$PATH_SEPARATOR$PATH])
AC_SUBST(UNIRECPROC)

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
