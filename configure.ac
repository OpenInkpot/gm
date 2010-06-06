AC_PREREQ(2.57)
AC_INIT(gm, 0.1, avn@openinkpot.org)
AM_INIT_AUTOMAKE(foreign)

AC_LANG_C
AC_PROG_CC

AM_PROG_LIBTOOL

PKG_CHECK_MODULES(EFL, [ecore ecore-file ecore-evas ecore-con evas edje efreet])
PKG_CHECK_MODULES(LIBCHOICEBOX,[libchoicebox])
PKG_CHECK_MODULES(LIBLANGUAGE,[liblanguage])
PKG_CHECK_MODULES(LIBKEYS, [libkeys])
PKG_CHECK_MODULES(LIBEOI, [libeoi])

AC_OUTPUT([Makefile src/Makefile po/Makefile data/Makefile
themes/Makefile themes/default/Makefile themes/azbooka/Makefile
])
