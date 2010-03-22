#ifndef _CHOICES_H
#define _CHOICES_H 1

bool
choicebox_pop(Evas_Object *choicebox);

typedef enum {
    CHOICEBOX_MAIN_MENU,
    CHOICEBOX_DEFAULT_SETTINGS,
    CHOICEBOX_GM_SETTINGS,
    CHOICEBOX_GM_APPS,
} choicebox_type_t;

Evas_Object *
choicebox_push(Evas_Object *parent, Evas *canvas,
    choicebox_handler_t handler,
    choicebox_draw_handler_t draw_handler,
    const char *name, int size, choicebox_type_t type, void *data);


#endif
