#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([libperseus_sdr], [0.2], [andrew@montefusco.com])
AC_CONFIG_SRCDIR([perseuserr.h])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

#
# configure platform
# On a x86 PC with linux running $host is: i686-pc-linux-gnu
#
#
case $host in

  *linux*)
    AC_MSG_NOTICE([Platform LINUX: $host])
    XXX_PRFX_DEF="/usr/local"
    export XXX_PRFX_DEF 
    ;;

  *darwin*)
    AC_MSG_NOTICE([Platform MAC OSX: $host])
    OSX_LDFLAGS=" -L/opt/local/lib"
    OSX_CFLAGS="-I/opt/local/include"
    LDFLAGS="$LDFLAGS $OSX_LDFLAGS"
    CFLAGS="$CFLAGS $OSX_CFLAGS"
    export LDFLAGS
    export CFLAGS
    XXX_PRFX_DEF="/opt/local"
    export XXX_PRFX_DEF 
    ;;

 *-*-*-*)
    AC_MSG_NOTICE([Platform unknown: $host])
    ;;

esac

AS_IF([test "x$XXX_PRFX_DEF" = x], AC_PREFIX_DEFAULT([`pwd`]), AC_PREFIX_DEFAULT(["$XXX_PRFX_DEF"]))

# Checks for libraries.

AC_CHECK_LIB([pthread], [pthread_create])
#AC_CHECK_LIB([usb-1.0], [libusb_init])
# FIXME: Replace `main' with a function in `-lusb-1.0':
AC_CHECK_LIB([usb-1.0], [libusb_init], [],
             [
              echo "Error! You need to have libusb-1.0 around."
              exit -1
             ])
AC_CHECK_LIB([dl], [dlopen])

#
# Check for utility used in code generation for FPGA
#
AC_CHECK_PROG(HAVE_HEXDUMP, hexdump, true, false)

if test "x$HAVE_HEXDUMP" = "xfalse"; then
	AC_MSG_ERROR([*********** hexdump needed but was not found, please install it (bsdmainutils package in Debian)])
fi


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

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

echo \
"-----------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
Now type 'make @<:@<target>@:>@'

 where the otpional <target> is:

    all           - build all binaries
    install       - install everything

-----------------------------------------------------"


