MAINTAINERCLEANFILES = Makefile.in

EDC= ${srcdir}/../gm.edc ${srcdir}/../items.edc
EDJ = $(EDC:%.edc=%.edj)

EXTRA_DIST = $(EDC)
CLEANFILES = $(EDJ)

themedir = $(pkgdatadir)/oi/themes/azbooka
theme_DATA = $(EDJ)

all: $(EDJ)
%.edj: %.edc
	edje_cc -id ${srcdir} -id  ${srcdir}/.. -o $@ $<

#FIXME: Remove this four lines after themes support enabling
install-exec-hook:
	mkdir -p ${DESTDIR}/usr/share/gm/themes
	ln -sf $(pkgdatadir)/oi/themes/azbooka/gm.edj ${DESTDIR}/usr/share/gm/themes/gm.edj
	ln -sf $(pkgdatadir)/oi/themes/azbooka/items.edj ${DESTDIR}/usr/share/gm/themes/items.edj
