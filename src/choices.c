#include <stdio.h>
#include <string.h>
#include <Evas.h>
#include <Edje.h>
#include <echoicebox.h>
#include "choices.h"

void
choicebox_pop(Evas_Object *choicebox)
{
    Evas_Object *parent;
    printf("Pop\n");
    Evas *canvas = evas_object_evas_get(choicebox);
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas, "main_canvas_edje");
    parent = evas_object_data_get(choicebox, "parent");
    if(!parent){
        printf("Not parent\n");
        return;
    }
    evas_object_hide(choicebox);
    edje_object_part_unswallow(main_canvas_edje,  choicebox);
    evas_object_del(choicebox);
    edje_object_part_text_set(main_canvas_edje, "footer", "");
    edje_object_part_text_set(main_canvas_edje, "path", "");
    edje_object_part_swallow(main_canvas_edje, "contents", parent);
    evas_object_focus_set(parent , true);
    evas_object_show(parent);
}

static void default_key_handler(void* param __attribute__((unused)),
        Evas* e __attribute__((unused)), Evas_Object *r, void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    fprintf(stderr, "kn: %s, k: %s, s: %s, c: %s\n", ev->keyname, ev->key, ev->string, ev->compose);
    choicebox_aux_key_down_handler(r, ev);

    if(!strcmp(ev->keyname, "Escape"))
    {
        choicebox_pop(r);
    }
}

static
void page_handler(Evas_Object* self,
                                int a,
                                int b,
                                void* param __attribute__((unused)))
{
    Evas *canvas = evas_object_evas_get(self);
    Evas_Object *main_edje = evas_object_name_find(canvas, "main_canvas_edje");
    printf("In page handler %d / %d\n", a, b);
    choicebox_aux_edje_footer_handler(main_edje, "footer", a, b);
}

Evas_Object *
choicebox_push(Evas_Object *parent, Evas *canvas,
    choicebox_handler_t handler,
    choicebox_draw_handler_t draw_handler,
    const char *name, int size, int own_edje, void *data)
{
    char *edje_file;
    if(own_edje)
        edje_file =  THEME_DIR "/items.edj";
    else
        edje_file = "/usr/share/echoicebox/echoicebox.edj";
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas, "main_canvas_edje");
    Evas_Object* choicebox = choicebox_new(canvas,
        edje_file,
        "full", handler, draw_handler, page_handler, data);
    if(!choicebox) {
         printf("no echoicebox\n");
        return NULL;
    }
    choicebox_set_size(choicebox, size);
    evas_object_name_set(choicebox, name);
    if(parent)
    {
        edje_object_part_unswallow(main_canvas_edje, parent);
        evas_object_hide(parent);
    }
    evas_object_data_set(choicebox, "parent", parent);
    edje_object_part_swallow(main_canvas_edje, "contents", choicebox);

    evas_object_focus_set(choicebox, true);
    evas_object_event_callback_add(choicebox,
                                  EVAS_CALLBACK_KEY_UP,
                                  &default_key_handler,
                                  data);
    evas_object_show(choicebox);
    return choicebox;
}
