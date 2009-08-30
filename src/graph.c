#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libintl.h>
#include <locale.h>
#include <time.h>

#include <Evas.h>
#include <Edje.h>

#include "graph.h"
#include "gm.h"
#include "run.h"
#include "raise.h"
#include "setup.h"

static void
gm_graphics_show_captions(Evas_Object *edje);

void
gm_graphics_show_clock(Evas *evas);

void
gm_graphics_show_book(Evas *evas);

static void
gm_graphics_show(Evas *evas) {
    Evas_Object * edje = evas_object_name_find(evas, "graphics");
    Evas_Object * main_edje = evas_object_name_find(evas, "main_window_edje");
    evas_object_hide(main_edje);
    gm_graphics_show_clock(evas);
    gm_graphics_show_book(evas);
    evas_object_show(edje);
    evas_object_focus_set(edje, 1);
}

static void
gm_graphics_hide(Evas *evas) {
    Evas_Object * edje = evas_object_name_find(evas, "graphics");
    Evas_Object * choicebox = evas_object_name_find(evas, "choicebox");
    Evas_Object * main_edje = evas_object_name_find(evas, "main_window_edje");
    evas_object_hide(edje);
    evas_object_show(main_edje);
    evas_object_focus_set(choicebox, 1);
}

static int _active = 0;

void
gm_graphics_activate(Evas *evas) {
    _active = 1;
    gm_graphics_show(evas);
}

static void
gm_graphics_deactivate(Evas *evas) {
    _active = 0;
    gm_graphics_hide(evas);
}

void
gm_graphics_conditional(Evas *evas) {
    if(_active)
        gm_graphics_show(evas);
}

void
gm_graphics_resize(Evas *evas, int x, int y) {
    Evas_Object * edje = evas_object_name_find(evas, "graphics");

    if(y > 600)
        edje_object_file_set(edje, THEME_DIR "/gm.edj", "vertical_graphics");
    else
        edje_object_file_set(edje, THEME_DIR "/gm.edj", "horizontal_graphics");

    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, x, y);

    gm_graphics_show_captions(edje);
    gm_graphics_show_book(evas);
    gm_graphics_show_clock(evas);
}

static void _keys_handler(void* param __attribute__((unused)),
                          Evas* e,
                          Evas_Object *r __attribute__((unused)),
                          void* event_info)
{
    const char* action = keys_lookup_by_event(gm_keys(), "graphical-menu",
                                              (Evas_Event_Key_Up*)event_info);

    if(!action)
        return;

    if(!strcmp(action, "TextMenu")) gm_graphics_deactivate(e);
    else if(!strcmp(action, "CurrentBook")) raise_fbreader(e);
    else if(!strcmp(action, "DateTimeSetup")) gm_run_etimetool(e);
    else if(!strcmp(action, "Books")) gm_run_madshelf_books(e);
    else if(!strcmp(action, "Images")) gm_run_madshelf_images(e);
    else if(!strcmp(action, "Audio")) gm_run_madshelf_audio(e);
    else if(!strcmp(action, "Games"))
    {
        gm_graphics_hide(e);
        gm_run_games(e);
    }
    else if(!strcmp(action, "Applications"))
    {
        gm_graphics_hide(e);
        gm_run_applications(e);
    }
    else if(!strcmp(action, "Settings"))
    {
        gm_graphics_hide(e);
        settings_menu(e);
    }
    else
        printf("Don't know how to handle action '%s'\n", action);
}

static void
gm_graphics_show_captions(Evas_Object *edje) {
    edje_object_part_text_set(edje, "caption_library", gettext("Library"));
    edje_object_part_text_set(edje, "caption_photo", gettext("Images"));
    char* s;
    asprintf(&s, "<inactive>%s</inactive>", gettext("Audio"));
    edje_object_part_text_set(edje, "caption_audio", s);
    free(s);
    edje_object_part_text_set(edje, "caption_games", gettext("Games"));
    edje_object_part_text_set(edje, "caption_apps", gettext("Applications"));
    edje_object_part_text_set(edje, "caption_setup", gettext("Setup"));
}

static void
_set_strftime(Evas_Object *edje, const char *part, const char *tmpl,
        struct tm * loctime) {
    char buf[256];
    strftime(buf, 256, tmpl, loctime);
    edje_object_part_text_set(edje, part, buf);
}

void
gm_graphics_show_clock(Evas *evas) {
    if(_active) {
       time_t curtime;
       struct tm * loctime;
       curtime = time (NULL);
       loctime = localtime (&curtime);
       Evas_Object *edje = evas_object_name_find(evas, "graphics");
       if(loctime->tm_year < 108) /* 2008 */
       {
           edje_object_part_text_set(edje, "caption_day", "--");
           edje_object_part_text_set(edje, "caption_dayofweek", "");
           edje_object_part_text_set(edje, "caption_month", "");
           edje_object_part_text_set(edje, "caption_clock", "-- : --");
       }
       else
       {
           _set_strftime(edje, "caption_day", "%d", loctime);
           _set_strftime(edje, "caption_dayofweek", "%A", loctime);
           _set_strftime(edje, "caption_month", "%B %Y", loctime);
           _set_strftime(edje, "caption_clock", "%H : %M", loctime);
           _set_strftime(edje, "caption_date", "%d.%m.%y", loctime);
       }
    }
}

void
gm_graphics_show_book(Evas *evas) {
    char buf[256];
    Evas_Object * edje = evas_object_name_find(evas, "graphics");
    if(_active) {
        struct bookinfo_t * bookinfo = gm_get_titles();
        if(bookinfo && bookinfo->title) {
            edje_object_part_text_set(edje, "caption_title", bookinfo->title);
            edje_object_part_text_set(edje, "caption_author",bookinfo->author);
            if(bookinfo->series_number) {
                snprintf(buf, 256, "%s #%d", bookinfo->series,
                    bookinfo->series_number);
                edje_object_part_text_set(edje, "caption_series", buf);
            }
            else
                edje_object_part_text_set(edje, "caption_series",
                    bookinfo->series);
        } else {
            edje_object_part_text_set(edje, "caption_title",
                                      gettext("<inactive>No book is open</inactive>"));
        }
    }
}

void
gm_graphics_init(Evas *evas) {
    Evas_Object *edje;
    edje = edje_object_add(evas);
    evas_object_name_set(edje, "graphics");

    int w, h;
    evas_output_size_get(evas, &w, &h);

    if(h > 600)
        edje_object_file_set(edje, THEME_DIR "/gm.edj", "vertical_graphics");
    else
        edje_object_file_set(edje, THEME_DIR "/gm.edj", "horizontal_graphics");

    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, w, h);

    evas_object_event_callback_add(edje,
                                  EVAS_CALLBACK_KEY_UP,
                                  &_keys_handler,
                                  evas);
    gm_graphics_show_captions(edje);
}


