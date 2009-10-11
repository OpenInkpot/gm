#include <stdio.h>
#include <string.h>
#include <Evas.h>
#include <Edje.h>
#include <libchoicebox.h>
#include <libeoi.h>
#include "choices.h"
#include "graph.h"
#include "gm.h"

#define DEFAULT_CHOICEBOX_THEME_FILE "/usr/share/choicebox/choicebox.edj"

bool
choicebox_pop(Evas_Object *choicebox)
{
    Evas_Object *parent;
    Evas *canvas = evas_object_evas_get(choicebox);
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas, "main_canvas_edje");
    parent = evas_object_data_get(choicebox, "parent");
    if(!parent){
        printf("Not parent\n");
        return false;
    }
    evas_object_hide(choicebox);
    edje_object_part_unswallow(main_canvas_edje,  choicebox);
    evas_object_del(choicebox);
    edje_object_part_text_set(main_canvas_edje, "footer", "");
    edje_object_part_text_set(main_canvas_edje, "path", "");
    edje_object_part_swallow(main_canvas_edje, "contents", parent);
    evas_object_focus_set(parent , true);
    evas_object_show(parent);
    Evas_Object *root = evas_object_name_find(canvas, "choicebox");
    if(root == parent)
    {
        gm_graphics_conditional(canvas);
        return false;
    }
    return true;
}

void
gm_choicebox_raise_root(Evas* canvas)
{
    Evas_Object *main_edje = evas_object_name_find(canvas, "main_canvas_edje");
    if(!main_edje)
        printf("No main edje\n");
    Evas_Object* box = edje_object_part_swallow_get(main_edje, "contents");
    while(choicebox_pop(box))
        box = edje_object_part_swallow_get(main_edje, "contents");

}

static
void page_handler(Evas_Object* self,
                                int a,
                                int b,
                                void* param __attribute__((unused)))
{
    Evas *canvas = evas_object_evas_get(self);
    Evas_Object *main_edje = evas_object_name_find(canvas, "main_canvas_edje");
    choicebox_aux_edje_footer_handler(main_edje, "footer", a, b);
}

static void close_handler(Evas_Object* choicebox __attribute__((unused)),
                          void *param __attribute__((unused)))
{
    choicebox_pop(choicebox);
}

Evas_Object *
choicebox_push(Evas_Object *parent, Evas *canvas,
    choicebox_handler_t handler,
    choicebox_draw_handler_t draw_handler,
    const char *name, int size, int own_edje, void *data)
{
    choicebox_info_t info = {
        NULL,
        DEFAULT_CHOICEBOX_THEME_FILE,
        "full",
        own_edje ? THEME_DIR "/items.edj" : DEFAULT_CHOICEBOX_THEME_FILE,
        own_edje ? "item-default" : "item-settings",
        handler,
        draw_handler,
        page_handler,
        close_handler,
    };

    Evas_Object *main_canvas_edje = evas_object_name_find(canvas, "main_canvas_edje");
    Evas_Object* choicebox = choicebox_new(canvas, &info, data);
    if(!choicebox) {
         printf("no choicebox\n");
        return NULL;
    }
    eoi_register_fullscreen_choicebox(choicebox);
    choicebox_set_size(choicebox, size);
    evas_object_name_set(choicebox, name);
    if(parent)
    {
        edje_object_part_unswallow(main_canvas_edje, parent);
        evas_object_hide(parent);
    }
    evas_object_data_set(choicebox, "parent", parent);
    edje_object_part_swallow(main_canvas_edje, "contents", choicebox);

    choicebox_aux_subscribe_key_up(choicebox);
    evas_object_focus_set(choicebox, true);
    evas_object_show(choicebox);
    return choicebox;
}
