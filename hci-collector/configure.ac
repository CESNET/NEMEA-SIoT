#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([bluetooth-hci-collector], [1.0], [nemea@cesnet.cz])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC


 AC_CONFIG_MACRO_DIR([../m4])

# Checks for libraries.
AC_CHECK_LIB([bluetooth], [hci_devid], [], [AC_MSG_ERROR([libbluetooth not found, install bluez-libs or its equivalent.])])

# Checks for header files.
AC_CHECK_HEADERS([sys/ioctl.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_CHECK_FUNCS([memset socket])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
