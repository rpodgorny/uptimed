dnl configure.in for uptimed

AC_INIT([uptimed], 0.3.14)
AM_CONFIG_HEADER(config.h)
AC_PROG_CC
AM_PROG_LIBTOOL
AM_INIT_AUTOMAKE

case "$host" in
  *-linux*)
    AC_DEFINE(PLATFORM_LINUX, 1, [Define if you are compiling for Linux])
    ;;
  *-hpux*)
    AC_DEFINE(PLATFORM_HPUX, 1, [Define if you are compiling for HP/UX])
    ;;
  *-solaris*)
    AC_DEFINE(PLATFORM_SOLARIS, 1, [Define if you are compiling for Solaris])
    ;;
  *-freebsd*)
    AC_DEFINE(PLATFORM_BSD, 1, [Define if you are compiling for *BSD])
    ;;
  *-bsdi*)
    AC_DEFINE(PLATFORM_BSD, 1, [Define if you are compiling for *BSD])
    ;;
  *-netbsd*)
    AC_DEFINE(PLATFORM_BSD, 1, [Define if you are compiling for *BSD])
    ;;
  *-openbsd*)
    AC_DEFINE(PLATFORM_BSD, 1, [Define if you are compiling for *BSD])
    ;;
  *-darwin*)
    AC_DEFINE(PLATFORM_BSD, 1, [Define if you are compiling for *BSD])
    ;;
  *)
    AC_DEFINE(PLATFORM_UNKNOWN, 1, [Define if you are compiling for an unknown system])
    ;;
esac

AC_REPLACE_FUNCS(getopt)
AC_CHECK_HEADERS(getopt.h)

AC_OUTPUT([Makefile libuptimed/Makefile src/Makefile man/Makefile etc/Makefile uptimed.spec])

echo ""
echo "--- Now type \"gmake\" to build Uptimed ---"
echo "--- If gmake is not working, try \"make\" ---"
echo ""
