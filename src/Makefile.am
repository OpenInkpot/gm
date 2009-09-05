bin_PROGRAMS = gm
gm_SOURCES = gm.c apps.c choices.c sock.c setup.c lang.c sound_control.c \
    screen_update_control.c raise.c clock.c battery.c rotation.c \
	graph.c run.c

AM_CFLAGS = --std=gnu99 -W -Wall -Werror \
    -DTHEME_DIR='"$(pkgdatadir)/themes/"'

INCLUDES = @EFL_CFLAGS@ @LIBCHOICEBOX_CFLAGS@ @LIBLANGUAGE_CFLAGS@ @LIBKEYS_CFLAGS@ @LIBEOI_CFLAGS@
gm_LDFLAGS = -Wl,--as-needed
gm_LDADD = @EFL_LIBS@ @LIBCHOICEBOX_LIBS@ @LIBLANGUAGE_LIBS@ @LIBKEYS_LIBS@ @LIBEOI_LIBS@
