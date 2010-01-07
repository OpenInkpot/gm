#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <libchoicebox.h>
#include <liblanguage.h>
#include "choices.h"


static languages_t *languages;

void *
init_langs() {
    languages = languages_get_supported();
    return languages; /* return void* only for checking and trhow error */
}

void
shutdown_langs()
{
    languages_free(languages);
}

const char *
current_lang()
{
    int i;
    for(i = 0; i < languages->n; ++i)
    {
        printf("lang.c: Compare %s and %s\n", languages->current,
            languages->langs[i].internal_name);
        if(!strcmp(languages->current, languages->langs[i].internal_name)){
            if(languages->langs[i].native_name)
                return languages->langs[i].native_name;
            if(languages->langs[i].name)
                return languages->langs[i].name;
            printf("Found lang with matching internal name\n");
        }
    }
    printf("lang.c: unknown language: %s\n", languages->current);
    return "Unknown";
}

static void lang_draw(Evas_Object *choicebox __attribute__((unused)),
                      Evas_Object *item,
                      int item_num,
                      int page_position __attribute__((unused)),
                      void *param __attribute__((unused)))
{
    language_t *lang = languages->langs + item_num;
    char *buf;
    if(lang->native_name)
        asprintf(&buf, "%s / %s", lang->native_name, lang->name);
    else
        buf = strdup(lang->name);
    edje_object_part_text_set(item, "title", buf);
    free(buf);
}

static void lang_handler(Evas_Object* choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param __attribute__((unused)))
{
    language_t *lang = languages->langs + item_num;
    languages_set(languages, lang->internal_name);
    ecore_main_loop_quit();
}

void lang_menu(Evas_Object *parent)
{
    Evas *canvas = evas_object_evas_get(parent);
    Evas_Object *choicebox;
    choicebox = choicebox_push(parent, canvas,
               lang_handler,
               lang_draw,
               "lang-choicebox", languages->n , 0, NULL);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "contents", "Select language");
}

