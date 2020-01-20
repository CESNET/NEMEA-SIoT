#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([zwave-detector], [1.0], [nemea@cesnet.cz])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

 AC_CONFIG_MACRO_DIR([../m4])

# Checks for libraries.
AX_LIBTRAP_CHECK()

# Checks for header files.
AC_CHECK_HEADERS([time.h sys/time.h ])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT64_T

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
