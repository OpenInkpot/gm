AC_PREREQ(2.57)
AC_INIT(gm, 0.1, avn@openinkpot.org)
AM_INIT_AUTOMAKE(foreign)

AC_LANG_C
AC_PROG_CC

PKG_CHECK_MODULES(EFL, [ecore ecore-file ecore-evas ecore-con evas edje efreet])
PKG_CHECK_MODULES(LIBCHOICEBOX,[libchoicebox])
PKG_CHECK_MODULES(LIBLANGUAGE,[language])
PKG_CHECK_MODULES(LIBKEYS, [libkeys])

AC_OUTPUT(Makefile src/Makefile themes/Makefile po/Makefile data/Makefile)
