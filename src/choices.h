#ifndef _CHOICES_H
#define _CHOICES_H 1

extern void
choicebox_pop(Evas_Object *choicebox);

Evas_Object *
choicebox_push(Evas_Object *parent, Evas *canvas,
    choicebox_handler_t handler,
    choicebox_draw_handler_t draw_handler,
    const char *name, int size, void *data); 
#endif
