#include <stdio.h>
#include <string.h>
#include <Ecore_Data.h>
#include <Evas.h>
#include <Ecore_File.h>
#include <Efreet.h>
#include <Edje.h>
#include <echoicebox.h>
#include "apps.h"

Ecore_List *
_load_desktops(const char * path, const char * category) {
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
        if(ecore_file_is_dir(filename))
            continue;
        current = efreet_desktop_get(filename);
        if (ecore_list_find(current->categories, 
            ECORE_COMPARE_CB(strcmp), category)) {
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
    edje_object_part_text_set(item, "choicebox/item/title", desktop->name); 
}

static void apps_page_handler(Evas_Object* choicebox __attribute__((unused)),
                                int a,
                                int b,
                                void* param __attribute__((unused)))
{
    printf("page: %d/%d\n", a, b);
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
    evas_object_del(choicebox);
}

void run_desktop_files(Evas *canvas, const char * path, const char * category) {
    Ecore_List * desktops = _load_desktops(path, category);
    Evas_Object* choicebox = choicebox_new(canvas, THEME_DIR "/gm.edj",
               "choicebox/item", apps_handler, 
               apps_draw_handler, apps_page_handler, (void *) desktops);
    evas_object_name_set(choicebox, "desktop-choicebox");
//    ecore_list_free(desktops);
}

void run_applications(void *canvas, void *category) {
    run_desktop_files((Evas *) canvas, "/usr/share/applications",
        (const char *) category);
}
