#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <libintl.h>
#include <echoicebox.h>
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
} rotation_t;

#define ROTATION_COUNT 5
rotation_t rotation_states[] = {
    { DISABLED, _("Disabled") },
    { _270, _("0 <-> 90 Degrees Clockwise") },
    { _180, _("0 <-> 180 Degrees") },
    { _90, _("0 <-> 90 Degrees Counterclockwise") },
    { CYCLE, _("0 -> 90 -> 180 -> 270 -> 0") }
};

const char *current_rotation()
{
    int i = CYCLE, j;
    FILE *f;

    f = fopen("/etc/default/erot", "r");
    if(f != NULL) {
        fscanf(f, "%d", &i);
        fclose(f);
    }

    for(j = 0; j < ROTATION_COUNT && rotation_states[j].value != i; j++);

    return gettext(rotation_states[j].text);
}

void set_rotation(int rotation)
{
    FILE *f;

    f = fopen("/etc/default/erot", "w");
    if(f != NULL) {
        fprintf(f, "%d", rotation);
        fclose(f);
    }
}

static void rotation_draw(Evas_Object* choicebox __attribute__((unused)),
                      Evas_Object* item,
                      int item_num,
                      int page_position __attribute__((unused)),
                      void* param __attribute__((unused)))
{
    edje_object_part_text_set(item, "text", gettext(rotation_states[item_num].text));
}

static void rotation_handler(Evas_Object* choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param __attribute__((unused)))
{
    set_rotation(rotation_states[item_num].value);
    choicebox_pop(choicebox);

    Evas_Object *parent = (Evas_Object*)param;
    choicebox_invalidate_item(parent, 1);
}

void rotation_menu(Evas_Object *parent) {
    Evas * canvas = evas_object_evas_get(parent);
    Evas_Object *choicebox;
    choicebox = choicebox_push(parent, canvas,
               rotation_handler,
               rotation_draw,
               "rotation-choicebox", ROTATION_COUNT , parent);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object * main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "contents", gettext("Select screen rotation type"));
}
