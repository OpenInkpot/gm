#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <libintl.h>
#include <libchoicebox.h>
#include "choices.h"

#define _(x) x

enum rotation {
    DISABLED = 0,
    _270 = 270,
    _180 = 180,
    _90 = 90,
    CYCLE = 360
};

typedef struct {
    int value;
    const char *text;
    const char *icon;
} rotation_t;

#define ROTATION_COUNT 5
rotation_t rotation_states[] = {
    { DISABLED, _("Disabled"), "set-icon-rotate-off" },
    { _270, _("0 \342\206\224 90 Degrees Clockwise"), "set-icon-rotate-cw" },
    { _180, _("0 \342\206\224 180 Degrees"), "set-icon-rotate-reverse" },
    { _90, _("0 \342\206\224 90 Degrees Counterclockwise"),
        "set-icon-rotate-ccw" },
    { CYCLE,
        _("0 \342\206\222 90 \342\206\222 180 \342\206\222 270 \342\206\222 0"),
        "set-icon-none"}
};

static int
current_rotation()
{
    int i = CYCLE, j;
    FILE *f;

    f = fopen("/etc/default/erot", "r");
    if(f != NULL) {
        fscanf(f, "%d", &i);
        fclose(f);
    }

    for(j = 0; j < ROTATION_COUNT && rotation_states[j].value != i; j++);
    return j;
}

const char *
gm_current_rotation()
{
    int j = current_rotation();
    return gettext(rotation_states[j].text);
}

void
gm_set_rotation_icon(Evas_Object *item)
{
    int j = current_rotation();
    edje_object_signal_emit(item, rotation_states[j].icon, "");
        printf("send: %s\n", rotation_states[j].icon);
}

static void
set_rotation(int rotation)
{
    FILE *f;

    f = fopen("/etc/default/erot", "w");
    if(f != NULL) {
        fprintf(f, "%d", rotation);
        fclose(f);
    }
}

static void rotation_draw(Evas_Object *choicebox __attribute__((unused)),
                      Evas_Object *item,
                      int item_num,
                      int page_position __attribute__((unused)),
                      void* param __attribute__((unused)))
{
    edje_object_part_text_set(item,
                              "lefttop",
                              gettext(rotation_states[item_num].text));
    edje_object_signal_emit(item, rotation_states[item_num].icon, "");
    printf("send: %s\n", rotation_states[item_num].icon);
}

static void rotation_handler(Evas_Object *choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void *param __attribute__((unused)))
{
    set_rotation(rotation_states[item_num].value);
    choicebox_pop(choicebox);

    Evas_Object *parent = (Evas_Object*)param;
    choicebox_invalidate_item(parent, 1);
}

void gm_rotation_menu(Evas_Object *parent)
{
    Evas *canvas = evas_object_evas_get(parent);
    Evas_Object *choicebox;
    choicebox = choicebox_push(parent, canvas,
               rotation_handler,
               rotation_draw,
               "rotation-choicebox", ROTATION_COUNT , 1, parent);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object * main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "contents", gettext("Select screen rotation type"));
}
