#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])

m4_define([package_version_major],[1])
m4_define([package_version_minor],[0])
m4_define([package_version_micro],[0])
m4_define([package_version_build],[2287])

AC_INIT([ixpdimm_sw], [package_version_major.package_version_minor.package_version_micro.package_version_build], [namratha.n.kothapalli@intel.com])
AC_CONFIG_SRCDIR([/src/cli/nvmcli.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules tar-pax no-dist-gzip dist-xz subdir-objects parallel-tests])
AM_SILENT_RULES([yes])

LT_PREREQ(2.2)
LT_INIT([disable-static])

AC_SUBST([PACKAGE_VERSION_MAJOR],[package_version_major])
AC_SUBST([PACKAGE_VERSION_MINOR],[package_version_minor])
AC_SUBST([PACKAGE_VERSION_MICRO],[package_version_micro])
AC_SUBST([PACKAGE_VERSION_BUILD],[package_version_build])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_CXX
AC_C_INLINE

# Checks for libraries.
PKG_CHECK_MODULES([SQLITE3], [sqlite3])
PKG_CHECK_MODULES([NDCTL], [libndctl >= 54])
PKG_CHECK_MODULES([INVM_CIM], [libinvm-cim])
PKG_CHECK_MODULES([INVM_CLI], [libinvm-cli])
PKG_CHECK_MODULES([INVM_I18N], [libinvm-i18n])
PKG_CHECK_MODULES([OPENSSL], [openssl])
PKG_CHECK_MODULES([LIBCRYPTO], [libcrypto])
PKG_CHECK_MODULES([ZLIB], [zlib])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h locale.h stddef.h stdlib.h string.h sys/param.h unistd.h])
AC_CHECK_HEADERS([limits.h syslog.h wchar.h])
AC_CHECK_HEADERS([libintl.h sys/ioctl.h termios.h])
AC_CHECK_HEADER(windows.h, AC_DEFINE(HAVE_WINDOWS_H, 1, [Compiling on Windows]))
AC_CHECK_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_FORK

AC_CHECK_FUNCS([getpagesize memset munmap])
AC_CHECK_FUNCS([memchr pow strcspn strtoull])
AC_CHECK_FUNCS([floor getcwd gethostname memmove mkdir realpath rmdir])
AC_CHECK_FUNCS([setlocale socket strchr strerror strstr strtol strtoul])
AC_CHECK_FUNCS([uname])

AC_CANONICAL_HOST
build_windows=no
case "${host_os}" in
        linux*)
		build_linux=yes
	;;
	cygwin*|mingw*)
		build_windows=yes
	;;
esac
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
