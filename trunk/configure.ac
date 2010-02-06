#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libperseus_sdr, 0.1, andrew@montefusco.com)
AC_CONFIG_SRCDIR([perseuserr.h])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE(perseustest, 0.1)

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

AC_CHECK_LIB([pthread], [pthread_create])
#AC_CHECK_LIB([usb-1.0], [libusb_init])
# FIXME: Replace `main' with a function in `-lusb-1.0':
AC_CHECK_LIB([usb-1.0], [libusb_init], [],
             [
              echo "Error! You need to have libusb-1.0 around."
              exit -1
             ])



# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday])

AC_OUTPUT(Makefile)

