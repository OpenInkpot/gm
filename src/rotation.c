#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <err.h>

#include <Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <Ecore_File.h>

#include <libchoicebox.h>
#include <gm-configlet.h>
#include "choices.h"

#define _(x) x

#define PATH_MAX 4096

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
        "set-icon-rotate-loop"}
};

static int
read_current_rotation()
{
    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "%s/.e/apps/erot/config", getenv("HOME"));
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        int i;
        fscanf(f, "%d", &i);
        fclose(f);
        return i;
    }

    f = fopen("/etc/default/erot", "r");
    if(f != NULL) {
        int i;
        fscanf(f, "%d", &i);
        fclose(f);
    }

    return CYCLE;
}

static int
current_rotation()
{
    int i = read_current_rotation();

    for(int j = 0; j < ROTATION_COUNT; j++)
        if (rotation_states[j].value == i)
            return j;

    return 4;
}

static const char *
gm_current_rotation()
{
    int j = current_rotation();
    return gettext(rotation_states[j].text);
}

static void
gm_set_rotation_icon(Evas_Object *item)
{
    int j = current_rotation();
    edje_object_signal_emit(item, rotation_states[j].icon, "");
    printf("sendxx: %s\n", rotation_states[j].icon);
}

static void
set_rotation(int rotation)
{
    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "%s/.e/apps/erot", getenv("HOME"));
    ecore_file_mkpath(filename);

    strcat(filename, "/config");
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        warn("%s", filename);
        return;
    }

    fprintf(stderr, "%d", rotation);

    fprintf(f, "%d", rotation);
    fclose(f);
}

static void rotation_submenu_draw(
                      Evas_Object *choicebox __attribute__((unused)),
                      Evas_Object *item,
                      int item_num,
                      int page_position __attribute__((unused)),
                      void* param __attribute__((unused)))
{
    edje_object_part_text_set(item,
                              "title",
                              gettext(rotation_states[item_num].text));
    edje_object_signal_emit(item, rotation_states[item_num].icon, "");
    printf("send: %s\n", rotation_states[item_num].icon);
}

static void rotation_submenu_handler(
                    Evas_Object *choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void *param __attribute__((unused)))
{
    set_rotation(rotation_states[item_num].value);
    gm_configlet_submenu_pop(choicebox);

    Evas_Object *parent = (Evas_Object*)param;
    choicebox_invalidate_item(parent, 1);
}

static void
gm_rotation_menu(void *data __attribute__((unused)), Evas_Object *parent)
{
    Evas *canvas = evas_object_evas_get(parent);
    Evas_Object *choicebox;
    choicebox = gm_configlet_submenu_push(parent,
               rotation_submenu_handler,
               rotation_submenu_draw,
               ROTATION_COUNT,
               NULL);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object * main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "title", gettext("Select screen rotation type"));
}

static void
rotation_draw(void *data __attribute__((unused)), Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Screen rotation type"));
    edje_object_part_text_set(item, "value", gm_current_rotation());
    gm_set_rotation_icon(item);
}

const configlet_plugin_t *
configlet_rotation(void)
{
    static const configlet_plugin_t configlet = {
        .load = NULL,
        .unload = NULL,
        .draw = rotation_draw,
        .select = gm_rotation_menu,
        .sort_key = "02rotation",
    };
    return &configlet;
}
