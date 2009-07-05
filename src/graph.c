#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <Evas.h>
#include <Edje.h>
#include "graph.h"
#include "gm.h"
#include "run.h"
#include "raise.h"

static void
gm_graphics_show(Evas *evas) {
    Evas_Object * edje = evas_object_name_find(evas, "graphics");
    Evas_Object * main_edje = evas_object_name_find(evas, "main_window_edje");
    Evas_Object * choicebox = evas_object_name_find(evas, "choicebox");
    evas_object_hide(main_edje);
    evas_object_hide(choicebox);
    evas_object_show(edje);
    evas_object_focus_set(edje, 1);
}

static void
gm_graphics_hide(Evas *evas) {
    Evas_Object * edje = evas_object_name_find(evas, "graphics");
    Evas_Object * choicebox = evas_object_name_find(evas, "choicebox");
    Evas_Object * main_edje = evas_object_name_find(evas, "main_window_edje");
    evas_object_hide(edje);
    evas_object_show(main_edje);
    evas_object_show(choicebox);
    evas_object_focus_set(choicebox, 1);
}

static int _active = 0;

void
gm_graphics_activate(Evas *evas) {
    _active = 1;
    gm_graphics_show(evas);
}

static void
gm_graphics_deactivate(Evas *evas) {
    _active = 0;
    gm_graphics_hide(evas);
}

void
gm_graphics_conditional(Evas *evas) {
    if(_active)
        gm_graphics_show(evas);
}

static void
gm_graphics_run(Evas *evas, int no) {
    gm_graphics_hide(evas);
    fake_main_menu_handler(evas, no);
}

void
gm_graphics_resize(Evas *evas, int x, int y) {
    Evas_Object * edje = evas_object_name_find(evas, "graphics");
    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, x, y);
}

static void
kp_activate(Evas *e, char k) {
    int kn = k - '0';
    switch(kn) {
        case 1:  raise_fbreader(e); break ;
        case 2:  gm_run_etimetool(e); break;
        case 3:  gm_run_madshelf_books(e); break;
        case 4:  gm_run_madshelf_images(e); break;
        case 6:  gm_graphics_run(e, 7); break;
        case 7:  gm_graphics_run(e, 6); break;
        case 8:  gm_graphics_run(e, 8); break;
    default:
        printf("Don't know how to handle %d\n", kn);
    }
}

static void _keys_handler(void* param __attribute__((unused)),
        Evas* e,
        Evas_Object *r __attribute__((unused)),
        void* event_info) {

    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    fprintf(stderr, "kn: %s, k: %s, s: %s, c: %s\n", ev->keyname, ev->key, ev->string, ev->compose);
    char *k = ev->keyname;
    if(!strcmp(k, "Escape"))
       gm_graphics_deactivate(e);
    if(!strncmp(k, "KP_", 3) && (isdigit(k[3])) && !k[4])
       kp_activate(e, k[3]);
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


