#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([libperseus_sdr], [0.3], [andrew@montefusco.com])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([perseuserr.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([build-aux])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL


#
# Checking for build cpu
# and, if 64 bit, add some linking option
#
case $build_cpu in

    i686)
      AC_MSG_NOTICE([Platform: ix86 32 bit])
      # checks for platform dependent compiler optimizations
      AX_CHECK_COMPILE_FLAG(-ffast-math, SIMD_FLAGS="$SIMD_FLAGS -ffast-math", [])
      AX_CHECK_COMPILE_FLAG(-fopenmp, SIMD_FLAGS="$SIMD_FLAGS -fopenmp", [])
      SIMD_FLAGS="$SIMD_FLAGS  -march=native "
      AC_SUBST(SIMD_FLAGS)
    ;;

    x86_64)
      AC_MSG_NOTICE([Platform: ix86 64 bit])
      LDFLAGS="$LDFLAGS -L/lib64 -L/usr/lib64"
      export LDFLAGS
      AC_MSG_NOTICE([${LDFLAGS}])      
      # checks for platform dependent compiler optimizations
      AX_CHECK_COMPILE_FLAG(-ffast-math, SIMD_FLAGS="$SIMD_FLAGS -ffast-math", [])
      AX_CHECK_COMPILE_FLAG(-fopenmp, SIMD_FLAGS="$SIMD_FLAGS -fopenmp", [])
      SIMD_FLAGS="$SIMD_FLAGS  -march=native "
      AC_SUBST(SIMD_FLAGS)
    ;;

    armv6l)
      AC_MSG_NOTICE([Platform: ARM 6])
      CFLAGS="-O3 -pipe -march=armv6 -mfpu=vfp -mfloat-abi=hard"
      CXXFLAGS="-O3 -pipe -march=armv6 -mfpu=vfp -mfloat-abi=hard"
      export CFLAGS
      export CXXFLAGS
      AC_MSG_NOTICE([${CFLAGS}])
      AC_MSG_NOTICE([${CXXFLAGS}])
    ;;

    armv7l)
      AC_MSG_NOTICE([Platform: ARM 7])
      CFLAGS="-O3 -pipe -march=armv7 -mfpu=neon -fno-tree-vectorize -fopenmp"
      CXXFLAGS="-O3 -pipe -march=armv7 -mfpu=neon -fno-tree-vectorize -fopenmp"
      export CFLAGS
      export CXXFLAGS
      AC_MSG_NOTICE([${CFLAGS}])
      AC_MSG_NOTICE([${CXXFLAGS}])
    ;;

esac

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

INST_USER=`echo $(whoami)`
AC_SUBST(INST_USER)


AS_IF([test "x$XXX_PRFX_DEF" = x], AC_PREFIX_DEFAULT([`pwd`]), AC_PREFIX_DEFAULT(["$XXX_PRFX_DEF"]))

# Checks for libraries.

AC_CHECK_LIB([pthread], [pthread_create])
#AC_CHECK_LIB([usb-1.0], [libusb_init])

AC_CHECK_LIB([usb-1.0], [libusb_init], [],
             [
              echo "Error! You need to have libusb-1.0 around."
              echo "Ubuntu/Debian: sudo apt-get install libusb-1.0-0-dev"
              echo "RedHat:        yum install libusb1 libusb1-devel"
              exit -1
             ])
AC_CHECK_LIB([dl], [dlopen])

#
# Check for utility used in code generation for FPGA
#
AC_CHECK_PROG(HAVE_HEXDUMP, hexdump, true, false)

if test "x$HAVE_HEXDUMP" = "xfalse"; then
	AC_MSG_ERROR([*********** hexdump needed but was not found, please install it (bsdmainutils package in Debian/Ubuntu)])
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

#
# check the compiler option really working
#
corw=`gcc -Q --help=target -march=native`

echo \
"-----------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
Now type 'make @<:@<target>@:>@'

 where the otpional <target> is:

    all           - build all binaries
    install       - install everything

${corw}

-----------------------------------------------------"


