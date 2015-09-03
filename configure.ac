
AC_INIT([eos-speedwagon], [0],
    [], [eos-speedwagon], [http://endlessm.com])
AM_INIT_AUTOMAKE([-Wall -Wno-portability foreign 1.11 tar-ustar])
AM_SILENT_RULES([yes])

AC_PROG_CC
PKG_PROG_PKG_CONFIG

GLIB_COMPILE_RESOURCES=`$PKG_CONFIG --variable glib_compile_resources gio-2.0`
AC_SUBST(GLIB_COMPILE_RESOURCES)

GDBUS_CODEGEN=`$PKG_CONFIG --variable gdbus_codegen gio-2.0`
AC_SUBST(GDBUS_CODEGEN)

PKG_CHECK_MODULES(SPEEDWAGON, [gtk+-3.0 gio-unix-2.0])

AC_CACHE_SAVE

AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
