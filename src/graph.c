#include <stdio.h>
#include <string.h>
#include <Evas.h>
#include <Edje.h>
#include "graph.h"

static void _keys_handler(void* param __attribute__((unused)),
        Evas* e,
        Evas_Object *r __attribute__((unused)), void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    fprintf(stderr, "kn: %s, k: %s, s: %s, c: %s\n", ev->keyname, ev->key, ev->string, ev->compose);
    if(!strcmp(ev->keyname, "Escape"))
       gm_graphics_hide(e);
}

void
gm_graphics_show(Evas *evas) {
    Evas_Object * edje = evas_object_name_find(evas, "graphics");
    Evas_Object * main_edje = evas_object_name_find(evas, "main_window_edje");
    Evas_Object * choicebox = evas_object_name_find(evas, "choicebox");
    evas_object_hide(main_edje);
    evas_object_hide(choicebox);
    evas_object_show(edje);
    evas_object_focus_set(edje, 1);
}

void
gm_graphics_hide(Evas *evas) {
    Evas_Object * edje = evas_object_name_find(evas, "graphics");
    Evas_Object * choicebox = evas_object_name_find(evas, "choicebox");
    Evas_Object * main_edje = evas_object_name_find(evas, "main_window_edje");
    evas_object_hide(edje);
    evas_object_show(main_edje);
    evas_object_show(choicebox);
    evas_object_focus_set(choicebox, 1);
}

void
gm_graphics_resize(Evas *evas, int x, int y) {
    Evas_Object * edje = evas_object_name_find(evas, "graphics");
    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, x, y);
}

void
gm_graphics_init(Evas *evas) {
    Evas_Object *edje;
    edje = edje_object_add(evas);
    evas_object_name_set(edje, "graphics");
    edje_object_file_set(edje, THEME_DIR "/gm.edj", "vertical_graphics");
    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, 600, 800);
    evas_object_event_callback_add(edje,
                                  EVAS_CALLBACK_KEY_UP,
                                  &_keys_handler,
                                  evas);
}


