THEMES=$(wildcard *.edc)
COMPILED_THEMES=$(patsubst %.edc,%.edj,$(THEMES))

themedir=$(datadir)/eoi/themes/azbooka
theme_DATA=$(COMPILED_THEMES)

%.edj: %.edc
	edje_cc -id .. -id . $< $@

CLEANFILES=$(COMPILED_THEMES)
