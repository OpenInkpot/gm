AC_PREREQ(2.57)
AC_INIT(gm, 0.1, avn@openinkpot.org)
AM_INIT_AUTOMAKE(foreign)
AM_CONFIG_HEADER(config.h)

AC_LANG_C
AC_PROG_CC

PKG_CHECK_MODULES(EFL, [ecore ecore-file ecore-evas ecore-con evas edje efreet])
PKG_CHECK_MODULES(ECHOICEBOX,[echoicebox])
PKG_CHECK_MODULES(LIBLANGUAGE,[language])

AC_OUTPUT(Makefile src/Makefile themes/Makefile po/Makefile)
