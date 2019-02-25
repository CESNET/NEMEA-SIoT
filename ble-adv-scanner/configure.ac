#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ble-adv-scanning], [1.0], [nemea@cesnet.cz])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.

AC_CONFIG_MACRO_DIR([../m4])

# Checks for libraries.
AX_PYTHON_MODULE(scapy,scapy,python3)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
