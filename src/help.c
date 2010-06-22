#include <libintl.h>
#include <libeoi.h>
#include <libeoi_help.h>
#include <Edje.h>
#include <libchoicebox.h>
#include "gm.h"
#include "graph.h"
#include "help.h"

#define HELP_WINDOW_ID "libeoi-help-window"

/* FIXME: this code know about help window internals */
void gm_help_close(Evas *canvas)
{
    Evas_Object *helpwin = evas_object_name_find(canvas, HELP_WINDOW_ID);
    if(helpwin)
    {
        Evas_Object *focus = evas_object_data_get(helpwin, "prev-focus");
        if (focus)
        {
            evas_object_show(focus);
            evas_object_focus_set(focus, true);
        }
        evas_object_hide(helpwin);
        evas_object_del(helpwin);
    }
    Evas_Object *mainwin = evas_object_name_find(canvas, "main_canvas_edje");
    if(mainwin)
    {
        evas_object_raise(mainwin);
        Evas_Object *choicebox =
            edje_object_part_swallow_get(mainwin, "contents");
        if(choicebox)
            choicebox_invalidate_item(choicebox, 0);
    }
    gm_graphics_conditional(canvas);
}

void gm_help_show(Evas* evas)
{
    eoi_help_show(evas, "gm", NULL, gettext("GM: Help"),
        gm_keys(), NULL);
}
