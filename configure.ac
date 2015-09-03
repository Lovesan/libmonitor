AC_INIT([libmonitor],[1.0],[lovesan.ru@gmail.com])

AC_CONFIG_SRCDIR([libmonitor/libmonitor.h])

AC_CONFIG_AUX_DIR([m4])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 foreign -Wall])

AM_PROG_AR

AC_PROG_CC

AC_PROG_CXX

AC_PROG_INSTALL

AC_SUBST([AM_CXXFLAGS], [-std=gnu++11])

LT_INIT

AC_CHECK_HEADERS([stdlib.h pthread.h time.h errno.h])

AC_TYPE_SIZE_T

AC_CHECK_FUNCS([calloc])

AC_CONFIG_FILES([Makefile libmonitor/Makefile libmonitorxx/Makefile libmonitorxx_test/Makefile])

AC_OUTPUT
