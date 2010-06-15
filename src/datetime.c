#include <libintl.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore.h>
#include "gm-configlet.h"

static void
datetime_draw(void *data __attribute__((unused)), Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Clock setup"));
    edje_object_part_text_set(item, "value", "");
    edje_object_signal_emit(item, "set-icon-time", "");
}

static void
datetime_set(void *data __attribute__((unused)),
             Evas_Object *item __attribute__((unused)))
{
    /* Not use gm_run_etimetool() here, because this code can be moved to
       etimetool package */
    Ecore_Exe *exe = ecore_exe_run("/usr/bin/etimetool --update-clock", NULL);
    if(exe)
        ecore_exe_free(exe);
}

const configlet_plugin_t *
configlet_datetime(void)
{
    static const configlet_plugin_t configlet = {
        .load = NULL,
        .unload = NULL,
        .draw = datetime_draw,
        .select = datetime_set,
        .sort_key = "04datetime",
    };
    return &configlet;
}
