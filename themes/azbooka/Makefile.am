MAINTAINERCLEANFILES = Makefile.in

EDC= ${srcdir}/../gm.edc ${srcdir}/../gm-items.edc
EDJ = gm.edj gm-items.edj

EXTRA_DIST = $(EDC)
CLEANFILES = $(EDJ)

themedir = $(datadir)/eoi/themes/azbooka
theme_DATA = $(EDJ)


all: $(EDJ)
gm-items.edj: ${srcdir}/../gm-items.edc
	edje_cc -id ${srcdir} -id  ${srcdir}/.. -o $@ $<

gm.edj: ${srcdir}/../gm.edc
	edje_cc -id ${srcdir} -id  ${srcdir}/.. -o $@ $<
