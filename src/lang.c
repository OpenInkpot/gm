#define _GNU_SOURCE
#include <libintl.h>
#include <stdio.h>
#include <string.h>
#include <Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <libchoicebox.h>
#include <liblanguage.h>
#include "gm-configlet.h"


static void *
init_langs() {
    return languages_get_supported();
}

static void
shutdown_langs(void *data)
{
    languages_t *languages = (languages_t *) data;
    languages_free(languages);
}

static const char *
current_lang(languages_t *languages)
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
                      void *param)
{
    languages_t *languages = param;
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
    languages_t *languages = param;
    language_t *lang = languages->langs + item_num;
    languages_set(languages, lang->internal_name);
    ecore_main_loop_quit();
}

static void language_set(void *data, Evas_Object *parent)
{
    languages_t *languages = data;
    Evas *canvas = evas_object_evas_get(parent);
    Evas_Object *choicebox;
    choicebox = gm_configlet_submenu_push(parent,
               lang_handler,
               lang_draw,
               languages->n,
               data);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "title", "Select language");
}

static void
language_draw(void *data, Evas_Object *item)
{
    languages_t *languages = data;
    /*
      TRANSLATORS: Please make this menu string two-language: 'Language
      (localized) <inactive>/ Language(in English)</inactive>'. This will allow
      users to reset language if current language is unknown to them translation
      is broken due to some reason, like lack of font).
    */
    edje_object_part_text_set(item, "title", gettext("Language <inactive>/ Language</inactive>"));
    edje_object_part_text_set(item, "value", current_lang(languages));
    edje_object_signal_emit(item, "set-icon-lang", "");
}


const configlet_plugin_t *
configlet_lang(void)
{
    static const configlet_plugin_t configlet = {
        .load = init_langs,
        .unload = shutdown_langs,
        .draw = language_draw,
        .select = language_set,
        .sort_key = "07lang",
    };
    return &configlet;
}
