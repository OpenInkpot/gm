#include <stdio.h>
#include <string.h>
#include <Evas.h>
#include <Edje.h>
#include <libchoicebox.h>
#include <libeoi.h>
#include <libkeys.h>
#include "choices.h"
#include "help.h"
#include "graph.h"
#include "gm.h"

#define DEFAULT_CHOICEBOX_THEME_FILE "choicebox"

static void help_key_handler(void *param __attribute__((unused)),
        Evas *e,
        Evas_Object *r __attribute__((unused)), void *event_info)
{
    const char *action = keys_lookup_by_event(gm_keys(), "text-menu",
                                              (Evas_Event_Key_Up*)event_info);
    if(action && !strcmp(action, "Help"))
        gm_help_show(e);
}

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
    char *title = evas_object_data_get(parent, "saved-title");
    edje_object_part_text_set(main_canvas_edje, "title", title ? title : "");
    if (title) {
        free(title);
        evas_object_data_set(parent, "saved-title", NULL);
    }
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
gm_choicebox_raise_root(Evas *canvas)
{
    Evas_Object *main_edje = evas_object_name_find(canvas, "main_canvas_edje");
    if(!main_edje)
        printf("No main edje\n");
    gm_help_close(canvas);
    Evas_Object *box = edje_object_part_swallow_get(main_edje, "contents");
    while(choicebox_pop(box))
        box = edje_object_part_swallow_get(main_edje, "contents");

}

static
void page_handler(Evas_Object *self,
                               int a,
                               int b,
                               void* param __attribute__((unused)))
{
    Evas *canvas = evas_object_evas_get(self);
    Evas_Object *main_edje = evas_object_name_find(canvas, "main_canvas_edje");
    choicebox_aux_edje_footer_handler(main_edje, "footer", a, b);
}

static void close_handler(Evas_Object *choicebox __attribute__((unused)),
                          void *param __attribute__((unused)))
{
    choicebox_pop(choicebox);
}

Evas_Object *
choicebox_push(Evas_Object *parent, Evas *canvas,
    choicebox_handler_t handler,
    choicebox_draw_handler_t draw_handler,
    const char *name, int size, choicebox_type_t type, void *data)
{
    char *theme = NULL; /* = NULL to shut up compiler warnings */
    char *group = NULL;
    if (type == CHOICEBOX_MAIN_MENU) {
        theme = "gm-items";
        group = "item-main-menu";
    } else if (type == CHOICEBOX_DEFAULT_SETTINGS) {
        theme = DEFAULT_CHOICEBOX_THEME_FILE;
        group = "item-settings";
    } else if (type == CHOICEBOX_GM_SETTINGS) {
        theme = "gm-items";
        group = "item-settings";
    } else if (type == CHOICEBOX_GM_APPS) {
        theme = "gm-items";
        group = "item-apps";
    }

    choicebox_info_t info = {
        NULL,
        DEFAULT_CHOICEBOX_THEME_FILE,
        "full",
        theme,
        group,
        handler,
        draw_handler,
        page_handler,
        close_handler,
    };

    Evas_Object *main_canvas_edje = evas_object_name_find(canvas, "main_canvas_edje");
    Evas_Object* choicebox = choicebox_new(canvas, &info, data);
    if(!choicebox)
    {
         printf("no choicebox\n");
        return NULL;
    }
    char *title = edje_object_part_text_get(main_canvas_edje, "title");
    if (parent && title) {
        evas_object_data_set(parent, "saved-title", strdup(title));
    }
    choicebox_set_size(choicebox, size);
    eoi_register_fullscreen_choicebox(choicebox);
    evas_object_name_set(choicebox, name);
    if(parent)
    {
        edje_object_part_unswallow(main_canvas_edje, parent);
        evas_object_hide(parent);
    }
    evas_object_data_set(choicebox, "parent", parent);
    edje_object_part_swallow(main_canvas_edje, "contents", choicebox);

    choicebox_aux_subscribe_key_up(choicebox);
    evas_object_event_callback_add(choicebox,
                                   EVAS_CALLBACK_KEY_UP,
                                   &help_key_handler,
                                   NULL);
    evas_object_focus_set(choicebox, true);
    evas_object_show(choicebox);
    evas_object_raise(choicebox);
    return choicebox;
}
