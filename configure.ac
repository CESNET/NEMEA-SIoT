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

AC_CONFIG_FILES([Makefile
                 hci-collector/Makefile
                 wsn-anomaly/detector/Makefile
                 ble-pairing/Makefile
                 lora-airtime/Makefile
                 lora-replay/Makefile])
AC_CONFIG_SUBDIRS([hci-collector
ble-pairing
wsn-anomaly/detector
lora-airtime
lora-replay])
AC_OUTPUT
