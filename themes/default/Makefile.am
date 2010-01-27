MAINTAINERCLEANFILES = Makefile.in

EDC= ${srcdir}/../gm.edc ${srcdir}/../items.edc
EDJ = $(EDC:%.edc=%.edj)

EXTRA_DIST = $(EDC)
CLEANFILES = $(EDJ)

themedir = $(pkgdatadir)/oi/themes/default
theme_DATA = $(EDJ)

all: $(EDJ)
%.edj: %.edc
	edje_cc -id ${srcdir} -id  ${srcdir}/.. -o $@ $<
