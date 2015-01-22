
AC_INIT([eos-speedwagon], [0],
    [], [eos-speedwagon], [http://endlessm.com])
AM_INIT_AUTOMAKE([-Wall -Wno-portability foreign 1.11 tar-ustar])
AM_SILENT_RULES([yes])

PKG_PROG_PKG_CONFIG

GJS_CONSOLE=`$PKG_CONFIG --variable=gjs_console gjs-1.0`
AC_SUBST(GJS_CONSOLE)

GLIB_COMPILE_RESOURCES=`$PKG_CONFIG --variable glib_compile_resources gio-2.0`
AC_SUBST(GLIB_COMPILE_RESOURCES)

AC_CACHE_SAVE

AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
