#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([nemea-siot], [1.0], [nemea@cesnet.cz])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign silent-rules subdir-objects])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

AC_CHECK_PROG(HAVE_PYTHON, python3, python3, [""])
AM_CONDITIONAL([HAVE_PYTHON], [test x$have_python = xyes])
if test x$have_python = xyes; then
  AC_DEFINE([HAVE_PYTHON], [1], [Define to 1 if the python3 is available])
else
  AC_DEFINE([HAVE_PYTHON], [0], [Define to 0 if the python3 is not available])
fi

# Checks for libraries.
AX_LIBTRAP_CHECK()
AX_UNIREC_CHECK()


# Checks for header files.
AC_CHECK_HEADERS([sys/ioctl.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([memset socket])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_SUBDIRS([hci-collector
ble-pairing
wsn-anomaly
lora-airtime
lora-replay
lora-distance
ble-adv-scanner])

AC_OUTPUT
