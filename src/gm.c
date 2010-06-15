#define _GNU_SOURCE 1
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libintl.h>
#include <locale.h>
#define _(x) x

#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <Efreet.h>
#include <Ecore_Con.h>

#include <libchoicebox.h>
#include <libeoi.h>
#include <libeoi_themes.h>
#include "lang.h"
#include "sock.h"
#include "choices.h"
#include "clock.h"
#include "gm.h"
#include "graph.h"
#include "raise.h"
#include "run.h"
#include "setup.h"
#include "apps_cleanup.h"
#include "help.h"

static keys_t *_gm_keys;

keys_t *gm_keys()
{
    if(!_gm_keys)
        _gm_keys = keys_alloc("gm");
    return _gm_keys;
}

struct main_menu_item {
    char *title;
    void (*execute)(Evas *evas_ptr);
    char *icon_signal;
};


struct main_menu_item main_menu[] = {
    {_("Current book"), &raise_fbreader, "set-icon-none" }, // Special
    {_("Library"), &gm_run_madshelf_books,  "set-icon-lib" },
    {_("Images"), &gm_run_madshelf_images,  "set-icon-photo"},
    {_("Audio"), &gm_run_madshelf_audio, "set-icon-phono"},
    {_("Games"), &gm_run_games , "set-icon-games"},
    {_("Applications"), &gm_run_applications, "set-icon-apps"},

    /*
      TRANSLATORS: Please make this menu string two-language:
      'Setup(localized) <inactive>/ Setup(in English)</inactive>'. This will
      allow users to reset language if current language is unknown to them
      (or translation is broken due to some reason, like lack of font).
    */
    {_("Setup <inactive>/ Setup</inactive>"), &settings_menu, "set-icon-setup"},
    {_("Recent books"), &gm_run_madshelf_recent, "set-icon-recent"},
    {_("Favorites"), &gm_run_madshelf_favorites, "set-icon-favorites"},
};

#define MAIN_MENU_SIZE (sizeof(main_menu)/sizeof(main_menu[0]))

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static int exit_handler(void *param __attribute__((unused)),
                        int ev_type __attribute__((unused)),
                        void *event __attribute__((unused)))
{
    ecore_main_loop_quit();
    return 1;
}

static void main_win_close_handler(Ecore_Evas *main_win __attribute__((unused)))
{
    ecore_main_loop_quit();
}

static void main_win_focus_in_handler(Ecore_Evas *main_win)
{
    Evas *canvas = ecore_evas_get(main_win);
    Evas_Object *choicebox = evas_object_name_find(canvas, "choicebox");
    if(choicebox)
        choicebox_invalidate_item(choicebox, 0);
    gm_graphics_show_book(canvas);
    if(getenv("GM_APPS_CLEANUP_ENABLE"))
        gm_apps_cleanup(main_win);
}

static int root_window_prop_change_handler(void *data,
        int type __attribute__((unused)), void *event)
{
    static Ecore_X_Atom atom = 0;
    static Ecore_X_Window prev_window = 0;
    const char *atom_name = "ACTIVE_DOC_WINDOW_ID";

    Ecore_X_Event_Window_Property *ev = event;
    Ecore_X_Window root = (Ecore_X_Window)data;

    if(atom == 0)
    {
        ecore_x_atom_get_prefetch(atom_name);
        ecore_x_atom_get_fetch();
        atom = ecore_x_atom_get(atom_name);
    }

    if(ev->win == root && ev->atom == atom)
    {
        Ecore_X_Window new_window = gm_get_active_document_window();
        if(prev_window != new_window && prev_window > 0)
            ecore_x_window_delete_request_send(prev_window);

        prev_window = new_window;
    }

    return 1;
}

static void draw_handler(Evas_Object *choicebox __attribute__((unused)),
                         Evas_Object *item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void *param __attribute__((unused)))
{
    /* All time formatting taken from libc manual, don't hurt me */
    char buf[256];
    struct bookinfo_t *bookinfo;

    /* blanking */
    edje_object_signal_emit(item, "set-icon-none", "");
    edje_object_part_text_set(item, "center-caption","");
    edje_object_part_text_set(item, "title","");
    edje_object_part_text_set(item, "author","");
    edje_object_part_text_set(item, "series","");
    edje_object_part_text_set(item, "progress","");

    /* Icon */
    if(item_num <= (int)MAIN_MENU_SIZE && main_menu[item_num].icon_signal)
        edje_object_signal_emit(item, main_menu[item_num].icon_signal, "");

    if((item_num == 0) && main_menu[item_num].title) {
        bookinfo = gm_get_titles();
        if(bookinfo && bookinfo->title) {
            edje_object_part_text_set(item, "title", bookinfo->title);
            edje_object_part_text_set(item, "author",bookinfo->author);
            if(bookinfo->series_number) {
                snprintf(buf, 256, "%s #%d", bookinfo->series,
                    bookinfo->series_number);
                edje_object_part_text_set(item, "series", buf);
            }
            else
                edje_object_part_text_set(item, "series",bookinfo->series);
            edje_object_signal_emit(item, "set-book", "");
        } else {
            char *str;
            asprintf(&str, "<inactive>%s</inactive>", gettext("No book is open"));
            edje_object_part_text_set(item, "center-caption", str);
            free(str);
            edje_object_signal_emit(item, "set-icon-no-book", "");
        }
        if (bookinfo->pages_count) {
            snprintf(buf, 256, "%d%%", 100 * bookinfo->current_page / bookinfo->pages_count);
            edje_object_part_text_set(item, "progress", buf);
        }
        gm_free_titles(bookinfo);
    } else if (main_menu[item_num].title) {
        edje_object_part_text_set(item, "center-caption", gettext(main_menu[item_num].title));
    }
}


static
void main_menu_handler(Evas_Object *choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void *param)
{
    main_menu[item_num].execute(param);
}

static void main_win_signal_handler(void *param,
        Evas_Object *o __attribute__((unused)),
        const char *emission __attribute__((unused)),
        const char *source __attribute__((unused)))
{
    Evas_Object *r = evas_object_name_find((Evas *) param, "choicebox");
    evas_object_del(r);
}

static void run(bool horizontal)
{
    Ecore_Evas *main_win = ecore_evas_software_x11_new(0, 0, 0, 0, horizontal ? 800 : 600, horizontal ? 600 : 800);
    gm_socket_server_start(main_win, "gm");
    ecore_evas_title_set(main_win, "GM");
    ecore_evas_name_class_set(main_win, "GM", "GM");

    Evas *main_canvas = ecore_evas_get(main_win);
    ecore_evas_callback_delete_request_set(main_win, main_win_close_handler);

    Evas_Object *main_canvas_edje = eoi_main_window_create(main_canvas);
    evas_object_name_set(main_canvas_edje, "main_canvas_edje");
    gm_init_clock_and_battery(main_canvas_edje, main_canvas);
    edje_object_signal_callback_add(main_canvas_edje, "*", "*",
        main_win_signal_handler, NULL);
    edje_object_part_text_set(main_canvas_edje, "footer", "");
    edje_object_part_text_set(main_canvas_edje, "title", "");
    evas_object_move(main_canvas_edje, 0, 0);
    evas_object_resize(main_canvas_edje, horizontal ? 800 : 600, horizontal? 600 :800);

    gm_settings_load();
    gm_graphics_init(main_canvas);

    Evas_Object *choicebox = choicebox_push(NULL, main_canvas,
        main_menu_handler, draw_handler, "choicebox", MAIN_MENU_SIZE, CHOICEBOX_MAIN_MENU,
        main_canvas);
    if(!choicebox) {
        printf("no choicebox\n");
        return;
    }

    Evas_Object *info = eoi_create_themed_edje(main_canvas, "gm", "text-menu");
    if (info) {
        char *title = edje_object_data_get(info, "title");
        if (title) {
            edje_object_part_text_set(main_canvas_edje, "title", title);
        }
        evas_object_del(info);
    }

    eoi_fullwindow_object_register(main_win, main_canvas_edje);
    ecore_evas_callback_focus_in_set(main_win, main_win_focus_in_handler);

    evas_object_show(main_canvas_edje);
    gm_graphics_conditional(main_canvas);
    ecore_evas_show(main_win);

    Ecore_X_Window root = ecore_x_window_root_first_get();
    ecore_x_event_mask_set(root, ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROPERTY,
         root_window_prop_change_handler, (void *)root);

    ecore_main_loop_begin();
}

static
void exit_all(void *param __attribute__((unused))) {
    ecore_main_loop_quit();
}

int main(int argc, char **argv __attribute__((unused)))
{
    setlocale(LC_ALL, "");
    textdomain("gm");
    if(!evas_init())
        die("Unable to initialize Evas\n");
    if(!ecore_init())
        die("Unable to initialize Ecore\n");
    if(!ecore_evas_init())
        die("Unable to initialize Ecore_Evas\n");
    if(!edje_init())
        die("Unable to initialize Edje\n");
    if(!efreet_init())
        die("Unable to initialize Efreet\n");
    if(!ecore_con_init())
        die("Unable to initialize Ecore_Con\n");

    ecore_x_io_error_handler_set(exit_all, NULL);
    ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);

    run(argc > 1);

    if(_gm_keys)
        keys_free(_gm_keys);

    gm_socket_server_stop();
    gm_settings_free();
    /* Keep valgrind happy */
    edje_file_cache_set(0);
    edje_collection_cache_set(0);

    ecore_con_shutdown();
    efreet_shutdown();
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
    edje_shutdown();
    return 0;
}
