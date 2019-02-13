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
python3 -c  'import scapy' 2>/dev/null
if test $? -eq 0
then
  AC_MSG_RESULT(Checking for scapy module...yes)
else
  AC_MSG_RESULT(Checking for scapy module...no)
  AC_MSG_WARN(["Scapy module missing."])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
