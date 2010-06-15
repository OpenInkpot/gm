bin_PROGRAMS = gm
gm_SOURCES = gm.c apps.c choices.c sock.c setup.c lang.c sound_control.c \
    raise.c clock.c battery.c \
	graph.c run.c apps_cleanup.c help.c user.c

AM_CFLAGS = --std=gnu99 -W -Wall \
    -DLOCALSTATEDIR='"$(localstatedir)"'

INCLUDES = @EFL_CFLAGS@ @LIBCHOICEBOX_CFLAGS@ @LIBLANGUAGE_CFLAGS@ @LIBKEYS_CFLAGS@ @LIBEOI_CFLAGS@
gm_LDFLAGS = -Wl,--as-needed -export-dynamic
gm_LDADD = @EFL_LIBS@ @LIBCHOICEBOX_LIBS@ @LIBLANGUAGE_LIBS@ @LIBKEYS_LIBS@ @LIBEOI_LIBS@

libconfigletdir = "${pkglibdir}/configlets"
libconfiglet_LTLIBRARIES = rotation.la screen.la version.la
rotation_la_SOURCES = rotation.c
rotation_la_LDFLAGS = -module -avoid-version -nostatic

screen_la_SOURCES = screen_update_control.c
screen_la_LDFLAGS = -module -avoid-version -nostatic

version_la_SOURCES = version.c
version_la_LDFLAGS = -module -avoid-version -nostatic @LIBLOPS_LIBS@ @EFL_LIBS@
