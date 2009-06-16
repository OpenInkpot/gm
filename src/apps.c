#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <Ecore_Data.h>
#include <Evas.h>
#include <Ecore_File.h>
#include <Efreet.h>
#include <Edje.h>
#include <echoicebox.h>
#include "choices.h"
#include "apps.h"

Ecore_List *
_load_desktops(const char * path, const char * category) {
    char fullname[1024];
    char *filename;
    Efreet_Desktop *current;
    Ecore_List *ls;
    Ecore_List * desktops = ecore_list_new();

    if(!desktops) {
        printf("out of memory\n");
        return desktops;
    }
    ecore_list_free_cb_set(desktops, ECORE_FREE_CB(efreet_desktop_free));

    ls = ecore_file_ls(path);
    if(!ls) {
        return desktops;
    };

    while ((filename = ecore_list_next(ls))) {
        snprintf(fullname, 1024, "%s/%s", path, filename);
        if(ecore_file_is_dir(fullname))
            continue;
        printf("Loading %s\n", filename);
        current = efreet_desktop_get(fullname);
        if(!current) {
            printf("Cam't load: %s\n", fullname);
            continue;
        }
        if (current->categories && ecore_list_find(current->categories,
            ECORE_COMPARE_CB(strcmp), category)
            && !current->no_display) {
                ecore_list_append(desktops, current);
                continue;
        };
        efreet_desktop_free(current);
    };
    ecore_list_destroy(ls);
    return desktops;
}

static void apps_draw_handler(Evas_Object* choicebox __attribute__((unused)),
                         Evas_Object* item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void* param)
{
    Efreet_Desktop  * desktop;
    desktop = ecore_list_index_goto((Ecore_List *) param, item_num);
    if (!desktop)
        return;
    edje_object_part_text_set(item, "text", desktop->name);
}

static void apps_handler(Evas_Object* choicebox,
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param)
{
    Efreet_Desktop  * desktop;
    desktop = ecore_list_index_goto((Ecore_List *) param, item_num);
    if (!desktop)
        return;
    efreet_desktop_exec(desktop, NULL, NULL);
    choicebox_pop(choicebox);
}

void run_desktop_files(Evas *canvas, const char * path, const char * category) {
    Evas_Object * choicebox;
    Ecore_List * desktops = _load_desktops(path, category);
    int count;
    if(!desktops) {
        printf("Can't read desktops\n");
        return;
    }
    Evas_Object * main_choicebox = evas_object_name_find(canvas, "choicebox");
    count =  ecore_list_count(desktops);
    printf("%d desktops loaded\n", count);
    choicebox = choicebox_push(main_choicebox, canvas,
               apps_handler,
               apps_draw_handler,
               "desktop-choicebox",
               count,
               (void *) desktops);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object * main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "path", category);
}

void run_applications(Evas *canvas, void *category) {
    category = gettext(category);
    run_desktop_files(canvas, "/usr/share/applications",
        (const char *) category);
}
