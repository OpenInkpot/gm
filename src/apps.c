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
    Evas * e = evas_object_evas_get(choicebox);
    Evas_Object *main_choicebox = evas_object_name_find(e, "choicebox");
    desktop = ecore_list_index_goto((Ecore_List *) param, item_num);
    if (!desktop)
        return;
    efreet_desktop_exec(desktop, NULL, NULL);
    evas_object_del(choicebox);
    evas_object_focus_set(main_choicebox, true);
    evas_object_show(main_choicebox);
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
        evas_object_del(obj);
        evas_object_focus_set(main_choicebox, true);
        evas_object_show(main_choicebox);
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
    choicebox = choicebox_new(canvas, THEME_DIR "/gm.edj",
               "choicebox/item", apps_handler, 
               apps_draw_handler, apps_page_handler, (void *) desktops);
    if(!choicebox)
        printf("We all dead\n");
    evas_object_name_set(choicebox, "desktop-choicebox");
    count =  ecore_list_count(desktops);
    printf("%d desktops loaded\n", count);
    choicebox_set_size(choicebox, count);
    evas_object_resize(choicebox, 600, 800);
    evas_object_move(choicebox, 0, 0);
    evas_object_hide(evas_object_name_find(canvas, "choicebox"));
    evas_object_show(choicebox);

    evas_object_focus_set(choicebox, true);
    evas_object_event_callback_add(choicebox,
                     EVAS_CALLBACK_KEY_DOWN,
                     &apps_choicebox_keys_callback,
                     NULL);
    evas_object_show(choicebox);
}

void run_applications(void *canvas, void *category) {
    run_desktop_files((Evas *) canvas, "/usr/share/applications",
        (const char *) category);
}
