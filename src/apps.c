#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <Eina.h>
#include <Evas.h>
#include <Ecore_File.h>
#include <Efreet.h>
#include <Edje.h>
#include <libchoicebox.h>
#include "choices.h"
#include "apps.h"

Eina_List *
_load_desktops(const char *path, const char *category) {
    char fullname[1024];
    char *filename;
    Efreet_Desktop *current;
    Eina_List *ls;
    Eina_List *ls_orig;
    Eina_List *l;
    char *data;
    Eina_List *desktops = NULL;

    ls_orig = ls = ecore_file_ls(path);
    if(!ls) {
        return desktops;
    };

    while(ls)
    {
        filename = eina_list_data_get(ls);
        ls = eina_list_next(ls);

        snprintf(fullname, 1024, "%s/%s", path, filename);
        if(ecore_file_is_dir(fullname))
            continue;
        current = efreet_desktop_get(fullname);
        if(!current)
        {
            printf("Cam't load: %s\n", fullname);
            continue;
        }
        if (current->categories && !current->no_display)
        {
            if(eina_list_search_unsorted(current->categories,
                                         EINA_COMPARE_CB(strcmp), category))
            {
                desktops = eina_list_append(desktops, current);
                continue;
            }
        }
        efreet_desktop_free(current);
    }

    EINA_LIST_FOREACH(ls_orig, l, data)
        free(data);
    eina_list_free(ls_orig);
    return desktops;
}

static void apps_draw_handler(Evas_Object *choicebox __attribute__((unused)),
                         Evas_Object *item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void *param)
{
    Efreet_Desktop  *desktop;
    if(!param)
        return;
    desktop = eina_list_nth((Eina_List *) param, item_num);
    if (!desktop)
        return;
    edje_object_part_text_set(item, "center-caption", desktop->name);

    if (desktop->icon) {
        fprintf(stderr, "%s\n", desktop->icon);
        edje_object_signal_emit(item, desktop->icon, "");
    }
}

static void apps_handler(Evas_Object *choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void *param)
{
    Efreet_Desktop  *desktop;
    if(!param)
        return;
    desktop = eina_list_nth((Eina_List *) param, item_num);
    if (!desktop)
        return;
    efreet_desktop_exec(desktop, NULL, NULL);
}

void run_desktop_files(Evas *canvas, const char *path, const char *category)
{
    Evas_Object *choicebox;
    Eina_List *desktops = _load_desktops(path, category);
    int count;
    Evas_Object *main_choicebox = evas_object_name_find(canvas, "choicebox");
    count =  eina_list_count(desktops);
    printf("%d desktops loaded\n", count);
    choicebox = choicebox_push(main_choicebox, canvas,
                   apps_handler,
                   apps_draw_handler,
                   "desktop-choicebox",
                   count,
                   CHOICEBOX_GM_APPS,
                   (void *) desktops);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas,
        "main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "title", gettext(category));
}

void run_applications(Evas *canvas, void *category) {

    run_desktop_files(canvas, "/usr/share/applications",
        (const char *) category);
}
