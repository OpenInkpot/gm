#define _GNU_SOURCE

#include <ctype.h>
#include <libintl.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <Evas.h>
#include <Edje.h>

#include <libeoi.h>
#include <libeoi_themes.h>

#include "graph.h"
#include "gm.h"
#include "run.h"
#include "raise.h"
#include "setup.h"
#include "help.h"

//#define DEBUG 1

static void
gm_graphics_cursor_set(Evas_Object *edje, const char *position);

static void
gm_graphics_show_captions(Evas_Object *edje);

void
gm_graphics_show_clock(Evas *evas);

void
gm_graphics_show_book(Evas *evas);

static void
gm_graphics_show(Evas *evas) {
    Evas_Object *edje = evas_object_name_find(evas, "graphics");
    Evas_Object *main_edje = evas_object_name_find(evas, "main_window_edje");
    evas_object_hide(main_edje);
    gm_graphics_show_clock(evas);
    evas_object_show(edje);
    gm_graphics_show_book(evas);
    evas_object_focus_set(edje, 1);
    evas_object_raise(edje);
}

static void
gm_graphics_hide_cover(Evas *evas)
{
    Evas_Object *image = evas_object_name_find(evas, "cover_image");
    if(image)
        evas_object_del(image);
    Evas_Object *border = evas_object_name_find(evas, "border");
    if(border)
        evas_object_del(border);
}

static void
gm_graphics_hide(Evas *evas) {
    Evas_Object *edje = evas_object_name_find(evas, "graphics");
    Evas_Object *choicebox = evas_object_name_find(evas, "choicebox");
    Evas_Object *main_edje = evas_object_name_find(evas, "main_window_edje");
    gm_graphics_hide_cover(evas);
    evas_object_hide(edje);
    evas_object_show(main_edje);
    evas_object_focus_set(choicebox, 1);
}

static int _active = true;

void
gm_graphics_mode_set(int value)
{
    _active = value;
}

int
gm_graphics_mode_get()
{
    return _active;
}

void
gm_graphics_conditional(Evas *evas)
{
    if(_active)
        gm_graphics_show(evas);
}

static void
gm_graphics_resize(Ecore_Evas *window __attribute__((unused)),
                   Evas_Object *edje, int x, int y,
                   void *param __attribute__((unused)))
{
    const char *file;
    const char *collection;
    char *replacement = (y > 600) ? "vertical_graphics" : "horizontal_graphics";

    edje_object_file_get(edje, &file, &collection);
    if(!strcmp(collection, replacement))
        return;
    edje_object_file_set(edje, file, replacement);

    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, x, y);

    Evas *evas = evas_object_evas_get(edje);
    gm_graphics_show_captions(edje);
    gm_graphics_show_book(evas);
    gm_graphics_show_clock(evas);
    gm_graphics_cursor_set(edje, NULL);
}

static bool _action(Evas *e, const char *action)
{
    if(!strcmp(action, "Book")) open_current_book(e);
    else if(!strcmp(action, "Date")) gm_run_etimetool(e);
    else if(!strcmp(action, "Books")) gm_run_madshelf_books(e);
    else if(!strcmp(action, "Favorites")) gm_run_madshelf_favorites(e);
    else if(!strcmp(action, "Recent")) gm_run_madshelf_recent(e);
    else if(!strcmp(action, "Images")) gm_run_madshelf_images(e);
    else if(!strcmp(action, "Audio")) gm_run_madshelf_audio(e);
    else if(!strcmp(action, "Games"))
    {
        gm_graphics_hide(e);
        gm_run_games(e);
    }
    else if(!strcmp(action, "Apps"))
    {
        gm_graphics_hide(e);
        gm_run_applications(e);
    }
    else if(!strcmp(action, "Setup"))
    {
        gm_graphics_hide(e);
        settings_menu(e);
    }
    else
        return false;
    return true;
}

static void
gm_graphics_cursor_set(Evas_Object *edje, const char *position)
{
    edje_object_signal_emit(edje, "cursor_set", position ? position : "default");
}

static void
_cursor_move(Evas *evas, const char *move)
{
    Evas_Object *edje = evas_object_name_find(evas, "graphics");
    edje_object_signal_emit(edje, "move", move);
}

static void
_activate(void *data __attribute__((unused)),
          Evas_Object *o, const char *emission __attribute__((unused)),
          const char *source)
{
    Evas *evas = evas_object_evas_get(o);
    _action(evas, source);
}

static void
_cursor_select(Evas *evas)
{
    Evas_Object *edje = evas_object_name_find(evas, "graphics");
    edje_object_signal_emit(edje, "select", "");
}

static void _keys_handler(void *param __attribute__((unused)),
                          Evas *e,
                          Evas_Object *r __attribute__((unused)),
                          void *event_info)
{
    const char *action = keys_lookup_by_event(gm_keys(), "graphical-menu",
                                              (Evas_Event_Key_Up*)event_info);

    if(!action)
        return;
    evas_event_freeze(e);
    if(!strcmp(action, "Help"))
        gm_help_show(e);
    else if(!strcmp(action, "CursorUp") || !strcmp(action, "CursorDown")  ||
            !strcmp(action, "CursorLeft")  || !strcmp(action, "CursorRight"))
    {
        _cursor_move(e, action);
    }
    else if(!strcmp(action, "CursorSelect"))
        _cursor_select(e);
    else if(!_action(e, action))
        printf("Don't know how to handle action '%s'\n", action);
    evas_event_thaw(e);
}

static void
gm_graphics_show_captions(Evas_Object *edje) {
    edje_object_part_text_set(edje, "caption_library", gettext("Library"));
    edje_object_part_text_set(edje, "caption_photo", gettext("Images"));
    edje_object_part_text_set(edje, "caption_audio", gettext("Audio"));
    edje_object_part_text_set(edje, "caption_games", gettext("Games"));
    edje_object_part_text_set(edje, "caption_apps", gettext("Applications"));
    edje_object_part_text_set(edje, "caption_setup", gettext("Setup"));
}

static void
_set_strftime(Evas_Object *edje, const char *part, const char *tmpl,
        struct tm *loctime) {
    char buf[256];
    strftime(buf, 256, tmpl, loctime);
    edje_object_part_text_set(edje, part, buf);
}

void
gm_graphics_show_clock(Evas *evas) {
    if(_active) {
       Evas_Object *edje = evas_object_name_find(evas, "graphics");
       time_t curtime;
       struct tm *loctime;
       curtime = time (NULL);
       loctime = localtime (&curtime);
       if(loctime->tm_year < 108) /* 2008 */
       {
           edje_object_signal_emit(edje, "hide-clock", "");

           /* still show dashes, because some skin not hide clock */
           edje_object_part_text_set(edje, "caption_day", "");
           edje_object_part_text_set(edje, "caption_dayofweek", "");
           edje_object_part_text_set(edje, "caption_month", "");
           edje_object_part_text_set(edje, "caption_clock", "--:--");
       }
       else
       {
           edje_object_signal_emit(edje, "show-clock", "");
           _set_strftime(edje, "caption_day", "%d", loctime);
           _set_strftime(edje, "caption_dayofweek", "%A", loctime);
           _set_strftime(edje, "caption_month", "%B %Y", loctime);
           _set_strftime(edje, "caption_clock", "%H:%M", loctime);
           _set_strftime(edje, "caption_date", "%d.%m.%y", loctime);
       }
    }
}


static void
gm_get_image_geom(const char *fn, int *iw, int *ih)
{
    Evas_Imaging_Image *im = evas_imaging_image_load(fn, NULL);
    if(im)
    {
        evas_imaging_image_size_get(im, iw, ih);
        evas_imaging_image_free(im);
    }
    else
        printf("Can't load %s\n", fn);
}

static const char *filename_pattern = "/tmp/gm-cover-image-XXXXXX";
static char *filename = NULL;

static void
gm_graphics_update_cover_image(struct bookinfo_t *bookinfo, Evas *evas)
{
    Evas_Object *image;
    Evas_Object *border;
    Evas_Object *design = evas_object_name_find(evas, "graphics");
    gm_graphics_hide_cover(evas);
    if(filename)
    {
        unlink(filename);
        free(filename);
    }
    if(bookinfo->cover_image && bookinfo->cover_size > 0)
    {
        edje_object_signal_emit(design, "book-cover", "");
        image = evas_object_image_add(evas);
        evas_object_color_set(image, 0xff, 0xff, 0xff, 0xff);
        evas_object_name_set(image, "cover_image");
        filename = strdup(filename_pattern);
        int fd = mkstemp(filename);
        if(fd > 0)
        {
            int rc = write(fd, bookinfo->cover_image, bookinfo->cover_size);
            close(fd);
            if(rc == bookinfo->cover_size)
            {
                int iw, ih;
                double ratio;
                int x, y, w, h;
                edje_object_part_geometry_get(design, "cover_image",
                                              &x, &y, &w, &h);
                gm_get_image_geom(filename, &iw, &ih);
//                printf("geom: x=%d y=%d w=%d h=%d iw=%d ih=%d\n",
//                    x, y, w, h, iw, ih);
                if ( ih > h)
                {
                    ratio = (double) ih / (double) h;
                    ih = h;  iw /= ratio;
                }
                else
                {
                    ratio = (double) iw / (double) w;
                    iw = w; ih /= ratio;
                }
                evas_object_stack_above(image, design);
                x = x + (w - iw) / 2;
                y = y + (h - ih) / 2;

                border = evas_object_rectangle_add(evas);
                evas_object_name_set(border, "border");
                evas_object_color_set(border, 0x33, 0x33, 0x33, 0xFF);
                evas_object_resize(border, iw, ih);
                evas_object_move(border, x, y);
                evas_object_show(border);
                evas_object_stack_above(image, border);
//                printf("border at %d %d (%d, %d)\n", x, y, iw, ih);

#define BORDER_SIZE 3
                x += BORDER_SIZE; y += BORDER_SIZE;
                iw -= BORDER_SIZE * 2 ; ih -= BORDER_SIZE * 2;
#undef BORDER_SIZE

                evas_object_resize(image, iw, ih);
                evas_object_move(image, x , y);
                evas_object_image_filled_set(image, 1);
                evas_object_image_load_size_set(image, iw, ih);
                evas_object_image_file_set(image, filename, NULL);
                evas_object_show(image);
//                printf("image at %d %d (%d, %d)\n", x, y, iw, ih);
            }
        }
        free(filename);
    }
    else
        edje_object_signal_emit(design, "book-no-cover", "");
}

void
gm_graphics_show_book(Evas *evas) {
    evas_event_freeze(evas);
    Evas_Object *edje = evas_object_name_find(evas, "graphics");
    if(_active && evas_object_visible_get(edje))
    {
        struct bookinfo_t *bookinfo = gm_get_titles();
        edje_object_part_text_set(edje, "caption_title", "");
        edje_object_part_text_set(edje, "caption_author", "");
        edje_object_part_text_set(edje, "caption_author_picture", "");
        edje_object_part_text_set(edje, "caption_title_picture", "");
        edje_object_part_text_set(edje, "caption_series", "");
        if(bookinfo && bookinfo->title)
        {
            edje_object_part_text_set(edje, "caption_title",
                bookinfo->title);
            edje_object_part_text_set(edje, "caption_author",
                bookinfo->author);
            gm_graphics_update_cover_image(bookinfo, evas);
        }
        else
        {
            gm_graphics_hide_cover(evas);
            edje_object_signal_emit(edje, "no-book", "");
            edje_object_part_text_set(edje, "caption_title",
                                      gettext("No book is open"));
        }
        gm_free_titles(bookinfo);
    }
    evas_event_thaw(evas);
}

void
gm_graphics_init(Evas *evas) {
    int w, h;
    evas_output_size_get(evas, &w, &h);
    Evas_Object *edje = eoi_create_themed_edje(evas, THEME_EDJE,
        (h > 600) ? "vertical_graphics" : "horizontal_graphics");
    evas_object_name_set(edje, "graphics");

    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, w, h);

    evas_object_event_callback_add(edje,
                                  EVAS_CALLBACK_KEY_UP,
                                  &_keys_handler,
                                  evas);

    edje_object_signal_callback_add(edje,
                                    "activate",
                                    "*",
                                    _activate,
                                    NULL);
    eoi_resize_object_register(ecore_evas_ecore_evas_get(evas),
                               edje,
                               gm_graphics_resize, NULL);

    edje_object_signal_emit(edje, "no-book", NULL);
    gm_graphics_show_captions(edje);
    gm_graphics_cursor_set(edje, NULL);
}

