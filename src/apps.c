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
    edje_object_part_text_set(item, "text", desktop->name); 
}

static void apps_page_handler(Evas_Object* self,
                                int a __attribute__((unused)),
                                int b,
                                void* param __attribute__((unused)))
{
    Evas *canvas = evas_object_evas_get(self);
    Evas_Object *main_edje = evas_object_name_find(canvas, "main_window_edje");
    choicebox_aux_edje_footer_handler(main_edje, "footer", a, b); 
}


static void apps_handler(Evas_Object* choicebox,
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param)
{
    Efreet_Desktop  * desktop;
    Evas * canvas = evas_object_evas_get(choicebox);
    Evas_Object * main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    Evas_Object *main_choicebox = evas_object_name_find(canvas, "choicebox");
    desktop = ecore_list_index_goto((Ecore_List *) param, item_num);
    if (!desktop)
        return;
    efreet_desktop_exec(desktop, NULL, NULL);
    evas_object_focus_set(main_choicebox, true);
    edje_object_part_swallow(main_canvas_edje, "contents", main_choicebox);
    edje_object_part_text_set(main_canvas_edje, "footer", "");
    edje_object_part_text_set(main_canvas_edje, "path", "");
    evas_object_show(main_choicebox);
    evas_object_del(choicebox);
}

static void apps_choicebox_keys_callback(void* param __attribute__((unused)),
        Evas* e __attribute__((unused)), Evas_Object *obj, void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    fprintf(stderr, "kn: %s, k: %s, s: %s, c: %s\n", ev->keyname, ev->key, ev->string, ev->compose);

    choicebox_aux_key_down_handler(obj, ev);

    if(!strcmp(ev->keyname, "Escape"))
    {
        Evas_Object *main_choicebox = evas_object_name_find(e, "choicebox");
        Evas_Object * main_canvas_edje = evas_object_name_find(e,
                "main_canvas_edje");
        evas_object_del(obj);
        edje_object_part_text_set(main_canvas_edje, "footer", "");
        edje_object_part_text_set(main_canvas_edje, "path", "");
        evas_object_focus_set(main_choicebox, true);
        evas_object_show(main_choicebox);
        edje_object_part_swallow(main_canvas_edje, "contents", main_choicebox);
    }
  
}

void run_desktop_files(Evas *canvas, const char * path, const char * category) {
    Evas_Object * choicebox;
    Ecore_List * desktops = _load_desktops(path, category);
    int count;
    if(!desktops) {
        printf("Can't read desktops\n");
        return;
    }
    choicebox = choicebox_new(canvas, "/usr/share/echoicebox/echoicebox.edj",
               "full", apps_handler, 
               apps_draw_handler, apps_page_handler, (void *) desktops);
    if(!choicebox)
        printf("We all dead\n");
    evas_object_name_set(choicebox, "desktop-choicebox");
    count =  ecore_list_count(desktops);
    printf("%d desktops loaded\n", count);
    choicebox_set_size(choicebox, count);
    Evas_Object * main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    Evas_Object * main_choicebox = evas_object_name_find(canvas, "choicebox");
    edje_object_part_unswallow(main_canvas_edje, main_choicebox);
    evas_object_hide(main_choicebox);
    edje_object_part_text_set(main_canvas_edje, "path", category);

    evas_object_focus_set(choicebox, true);
    evas_object_event_callback_add(choicebox,
                     EVAS_CALLBACK_KEY_DOWN,
                     &apps_choicebox_keys_callback,
                     NULL);
    edje_object_part_swallow(main_canvas_edje, "contents", choicebox);
}

void run_applications(void *canvas, void *category) {
    run_desktop_files((Evas *) canvas, "/usr/share/applications",
        (const char *) category);
}
