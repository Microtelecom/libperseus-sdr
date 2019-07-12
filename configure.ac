#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([libperseus_sdr],
        [m4_esyscmd([./build-aux/git-version-gen .tarball-version])],
        [andrew@montefusco.com])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([perseus-sdr.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([build-aux])
AM_SILENT_RULES

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

case $host in
*-linux* | *-uclinux*)
	AC_MSG_RESULT([Linux])
	backend="linux"
	threads="posix"
	;;
*-darwin*)
	AC_MSG_RESULT([Darwin/Mac OS X])
	backend="darwin"
	threads="posix"
	;;
*-openbsd*)
	AC_MSG_RESULT([OpenBSD])
	backend="openbsd"
	threads="posix"
	;;
*-netbsd*)
	AC_MSG_RESULT([NetBSD])
	backend="netbsd"
	threads="posix"
	;;
*-mingw*)
	AC_MSG_RESULT([Windows])
	backend="windows"
	threads="posix"
	create_import_lib="yes"
	####AM_CFLAGS="${AM_CFLAGS} -fno-omit-frame-pointer"
	;;
*-cygwin*)
	AC_MSG_RESULT([Cygwin (using Windows backend)])
	backend="windows"
	threads="posix"
	;;
*-haiku*)
	AC_MSG_RESULT([Haiku])
	AC_CONFIG_SUBDIRS([libusb/os/haiku])
	backend="haiku"
	threads="posix"
	;;
*)
	AC_MSG_ERROR([unsupported operating system])
esac

#
# configure platform
# On a x86 PC with linux running $host is: i686-pc-linux-gnu
#
#
case $backend in

  *linux*)
    AC_MSG_NOTICE([Platform LINUX: $host])
    AC_DEFINE(OS_LINUX, 1, [Linux backend])
    AC_SUBST(OS_LINUX)

    XXX_PRFX_DEF="/usr/local"
    export XXX_PRFX_DEF

    # Checks for libraries.

    AC_CHECK_LIB([pthread], [pthread_create])
    AC_CHECK_LIB([usb-1.0], [libusb_init])

    AC_CHECK_LIB([usb-1.0], [libusb_init], [],
    [
         echo "Error! You need to have libusb-1.0 around."
         echo "Ubuntu/Debian: sudo apt-get install libusb-1.0-0-dev"
         echo "RedHat:        yum install libusb1 libusb1-devel"
         exit -1
    ])
    AC_CHECK_LIB([dl], [dlopen])

    # check for specific functions
    AC_CHECK_FUNCS([libusb_strerror], [],[])

    AC_MSG_NOTICE([Platform LINUX])
    ;;

  *darwin*)
    AC_MSG_NOTICE([Platform MAC OSX: $host])
    AC_DEFINE(OS_MACOS, 1, [macOS backend])
    AC_SUBST(OS_MACOS)
    OSX_LDFLAGS=" -L/opt/local/lib"
    OSX_CFLAGS="-I/opt/local/include"
    LDFLAGS="$LDFLAGS $OSX_LDFLAGS"
    CFLAGS="$CFLAGS $OSX_CFLAGS"
    export LDFLAGS
    export CFLAGS
    XXX_PRFX_DEF="/opt/local"
    export XXX_PRFX_DEF

    # Checks for libraries.

    AC_CHECK_LIB([pthread], [pthread_create])
    AC_CHECK_LIB([usb-1.0], [libusb_init])

    AC_CHECK_LIB([usb-1.0], [libusb_init], [],
    [
         echo "Error! You need to have libusb-1.0 around."
         exit -1
    ])
    AC_CHECK_LIB([dl], [dlopen])

    # check for specific functions
    AC_CHECK_FUNCS([libusb_strerror], [],[])

    AC_MSG_NOTICE([Platform MAC OSX])
    ;;

  *windows*)
    AC_DEFINE(OS_WINDOWS, 1, [Windows backend])
    AC_SUBST(OS_WINDOWS)
    #LIBS=""
    LTLDFLAGS="${LTLDFLAGS} -avoid-version -Wl,--add-stdcall-alias"

    #CFLAGS="$CFLAGS -I../libusb-1.0.last/"
    #CXXFLAGS="$CXXFLAGS -I../libusb-1.0.last/"
    LDFLAGS="$LDFLAGS -L/usr/lib/ -L/mingw32/bin/"

    AC_CHECK_HEADERS([libusb-1.0/libusb.h])

    #### AC_CHECK_LIB([usb-1.0], [libusb_init])
    AC_CHECK_LIB([usb-1.0], [libusb_init], [],
    [
         echo "Error! You need to have libusb-1.0 around."
		 echo "Take a look at README.Windows"
         exit -1
    ])

    # check for specific functions
    AC_CHECK_FUNCS([libusb_strerror], [],[])


    AC_DEFINE([POLL_NFDS_TYPE],[unsigned int],[type of second poll() argument])
    AC_MSG_NOTICE([Platform Windows])
    ;;

 *-*-*-*)
    AC_MSG_NOTICE([Platform unknown: $backend])
    ;;

esac

AS_IF([test "x$XXX_PRFX_DEF" = x], AC_PREFIX_DEFAULT([`pwd`]), AC_PREFIX_DEFAULT(["$XXX_PRFX_DEF"]))


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
      ARM_FLAGS=""
      AX_CHECK_COMPILE_FLAG(-march=armv6, ARM_FLAGS="$ARM_FLAGS -march=armv6", [])
      AX_CHECK_COMPILE_FLAG(-mfpu=vfp, ARM_FLAGS="$ARM_FLAGS -mfpu=vfp", [])
      AX_CHECK_COMPILE_FLAG(-mfloat-abi=hard, ARM_FLAGS="$ARM_FLAGS -mfloat-abi=hard", [])
      CFLAGS="-O3 -pipe $ARM_FLAGS"
      CXXFLAGS="-O3 -pipe $ARM_FLAGS"
      export CFLAGS
      export CXXFLAGS
      AC_MSG_NOTICE([${CFLAGS}])
      AC_MSG_NOTICE([${CXXFLAGS}])
    ;;

    armv7l)
      AC_MSG_NOTICE([Platform: ARM 7])
      ARM_FLAGS=""
      AX_CHECK_COMPILE_FLAG(-march=armv7, ARM_FLAGS="$ARM_FLAGS -march=armv7", [])
      AX_CHECK_COMPILE_FLAG(-mfpu=neon, ARM_FLAGS="$ARM_FLAGS -mfpu=neon", [])
      AX_CHECK_COMPILE_FLAG(-fno-tree-vectorize, ARM_FLAGS="$ARM_FLAGS -fno-tree-vectorize", [])
      AX_CHECK_COMPILE_FLAG(-fopenmp, ARM_FLAGS="$ARM_FLAGS -fopenmp", [])
      CFLAGS="-O3 -pipe $ARM_FLAGS"
      CXXFLAGS="-O3 -pipe $ARM_FLAGS"
      export CFLAGS
      export CXXFLAGS
      AC_MSG_NOTICE([${CFLAGS}])
      AC_MSG_NOTICE([${CXXFLAGS}])
    ;;

esac


INST_USER=`echo $(whoami)`
AC_SUBST(INST_USER)

#
# defines macros to be used in Makefile.am
#
AM_CONDITIONAL(OS_LINUX, test "x$backend" = xlinux)
AM_CONDITIONAL(OS_MACOS, test "x$backend" = xdarwin)
AM_CONDITIONAL(OS_WINDOWS, test "x$backend" = xwindows)



#
# Check for utility used in code generation for FPGA
#
AC_CHECK_PROG(HAVE_XXD, xxd, true, false)

if test "x$HAVE_XXD" = "xfalse"; then
	AC_MSG_ERROR([xxd needed but was not found, please install it (bsdmainutils package in Debian/Ubuntu)])
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

AC_CONFIG_FILES([Makefile
				 examples/Makefile
				 libperseus-sdr.pc
])

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
