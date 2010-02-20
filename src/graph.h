#ifndef _GRAPH_H
#define _GRAPH_H 1

extern void
gm_graphics_mode_set(int value);

extern int
gm_graphics_mode_get();

extern void
gm_graphics_conditional(Evas *evas);

extern void
gm_graphics_show_book(Evas *evas);

extern void
gm_graphics_show_clock(Evas *evas);

extern void
gm_graphics_init(Evas *evas);

#endif
