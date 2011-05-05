include_HEADERS = gm-configlet.h
bin_PROGRAMS = gm
gm_SOURCES = gm.c apps.c choices.c sock.c setup.c sound_control.c \
    raise.c clock.c battery.c graph.c run.c apps_cleanup.c help.c

AM_CFLAGS = --std=gnu99 -W -Wall \
    -DLOCALSTATEDIR='"$(localstatedir)"'

INCLUDES = @EFL_CFLAGS@ @LIBCHOICEBOX_CFLAGS@ @LIBLANGUAGE_CFLAGS@ @LIBKEYS_CFLAGS@ @LIBEOI_CFLAGS@ @LIBLOPS_CFLAGS@
gm_LDFLAGS = -Wl,--as-needed -export-dynamic
gm_LDADD = @EFL_LIBS@ @LIBCHOICEBOX_LIBS@ @LIBKEYS_LIBS@ @LIBEOI_LIBS@ 

libconfigletdir = "${pkglibdir}/configlets"
libconfiglet_LTLIBRARIES = rotation.la screen.la autorefresh.la version.la datetime.la \
						   lang.la user.la
rotation_la_SOURCES = rotation.c
rotation_la_LDFLAGS = -module -avoid-version -nostatic @EFL_LIBS@ 

screen_la_SOURCES = screen_update_control.c
screen_la_LDFLAGS = -module -avoid-version -nostatic @EFL_LIBS@ @LIBLOPS_LIBS@

autorefresh_la_SOURCES = screen_autorefresh_control.c
autorefresh_la_LDFLAGS = -module -avoid-version -nostatic @EFL_LIBS@ @LIBLOPS_LIBS@

version_la_SOURCES = version.c
version_la_LDFLAGS = -module -avoid-version -nostatic @EFL_LIBS@ @LIBLOPS_LIBS@

datetime_la_SOURCES = datetime.c
datetime_la_LDFLAGS = -module -avoid-version -nostatic @EFL_LIBS@ 

lang_la_SOURCES = lang.c
lang_la_LDFLAGS = -module -avoid-version -nostatic @EFL_LIBS@ @LIBLANGUAGE_LIBS@ 

user_la_SOURCES = user.c
user_la_LDFLAGS = -module -avoid-version -nostatic @EFL_LIBS@ 
