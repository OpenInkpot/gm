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

static keys_t* _gm_keys;

keys_t* gm_keys()
{
    if(!_gm_keys)
        _gm_keys = keys_alloc("gm");
    return _gm_keys;
}

struct main_menu_item {
    char *title;
    void (*execute)(Evas *evas_ptr);
    char * icon_signal;
};

void stub(Evas * e __attribute__((unused))) {
    printf("Stub\n");
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
};

#define MAIN_MENU_SIZE (sizeof(main_menu)/sizeof(main_menu[0]))

static void die(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static int exit_handler(void* param __attribute__((unused)),
                        int ev_type __attribute__((unused)),
                        void* event __attribute__((unused)))
{
    ecore_main_loop_quit();
    return 1;
}

static void main_win_close_handler(Ecore_Evas* main_win __attribute__((unused)))
{
    ecore_main_loop_quit();
}

static void main_win_focus_in_handler(Ecore_Evas* main_win)
{
    Evas *canvas = ecore_evas_get(main_win);
    Evas_Object *choicebox = evas_object_name_find(canvas, "choicebox");
    if(choicebox)
        choicebox_invalidate_item(choicebox, 0);
    gm_graphics_show_book(canvas);
    if(getenv("GM_APPS_CLEANUP_ENABLE"))
        gm_apps_cleanup(main_win);
}

static void draw_handler(Evas_Object* choicebox __attribute__((unused)),
                         Evas_Object* item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void* param __attribute__((unused)))
{
    /* All time formatting taken from libc manual, don't hurt me */
    char buf[256];
    struct bookinfo_t *bookinfo;

    if(item_num <= (int)MAIN_MENU_SIZE && main_menu[item_num].icon_signal)
        edje_object_signal_emit(item, main_menu[item_num].icon_signal, "");
    else
        edje_object_signal_emit(item, "set-icon-none", "");

    edje_object_part_text_set(item, "text","");
    edje_object_part_text_set(item, "lefttop","");
    edje_object_part_text_set(item, "leftbottom","");
    edje_object_part_text_set(item, "rightbottom","");

    if((item_num == 0) && main_menu[item_num].title ) {
        bookinfo = gm_get_titles();
        if(bookinfo && bookinfo->title) {
            edje_object_part_text_set(item, "text", bookinfo->title);
            edje_object_part_text_set(item, "lefttop",bookinfo->author);
            if(bookinfo->series_number) {
                snprintf(buf, 256, "%s #%d", bookinfo->series,
                    bookinfo->series_number);
                edje_object_part_text_set(item, "leftbottom", buf);
            }
            else
                edje_object_part_text_set(item, "leftbottom",bookinfo->series);
        } else {
            edje_object_part_text_set(item, "text",
                gettext("<inactive>No book is open</inactive>"));
        }
        gm_free_titles(bookinfo);
    } else
    if (main_menu[item_num].title) {
        edje_object_part_text_set(item, "text",
        gettext(main_menu[item_num].title));
    }

}


static
void main_menu_handler(Evas_Object* choicebox __attribute__((unused)),
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param)
{
    main_menu[item_num].execute(param);
}

static void main_win_resize_handler(Ecore_Evas* main_win)
{
    Evas* canvas = ecore_evas_get(main_win);
    int w, h;
    evas_output_size_get(canvas, &w, &h);

    eoi_process_resize(main_win);

    Evas_Object* main_canvas_edje = evas_object_name_find(canvas, "main_canvas_edje");
    evas_object_resize(main_canvas_edje, w, h);
    gm_graphics_resize(canvas, w, h);

    help_resize(canvas, w, h);
}


static void main_win_signal_handler(void* param,
        Evas_Object* o __attribute__((unused)),
        const char* emission __attribute__((unused)),
        const char* source __attribute__((unused)))
{
    Evas_Object* r = evas_object_name_find((Evas *) param, "choicebox");
    evas_object_del(r);
}

static void main_win_key_handler(void* param __attribute__((unused)),
        Evas* e __attribute__((unused)),
        Evas_Object *r __attribute__((unused)), void* event_info)
{
    const char* action = keys_lookup_by_event(gm_keys(), "text-menu",
                                              (Evas_Event_Key_Up*)event_info);

    if(action && !strcmp(action, "GraphicalMenu"))
        gm_graphics_activate(e);
    else if(action && !strcmp(action, "Help"))
        help_show(e);
}

static void run()
{
    Ecore_Evas* main_win = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
    gm_socket_server_start(main_win, "gm");
    ecore_evas_title_set(main_win, "GM");
    ecore_evas_name_class_set(main_win, "GM", "GM");

    Evas* main_canvas = ecore_evas_get(main_win);
    ecore_evas_callback_delete_request_set(main_win, main_win_close_handler);
    Evas_Object* main_canvas_edje = edje_object_add(main_canvas);
    evas_object_name_set(main_canvas_edje, "main_canvas_edje");
    edje_object_file_set(main_canvas_edje, THEME_DIR "/gm.edj", "main_window");
    gm_init_clock_and_battery(main_canvas_edje, main_canvas);
    edje_object_signal_callback_add(main_canvas_edje, "*", "*", main_win_signal_handler, NULL);
    edje_object_part_text_set(main_canvas_edje, "footer", "");
    edje_object_part_text_set(main_canvas_edje, "path", "");
    evas_object_move(main_canvas_edje, 0, 0);
    evas_object_resize(main_canvas_edje, 600, 800);

    gm_graphics_init(main_canvas);

    Evas_Object* choicebox = choicebox_push(NULL, main_canvas,
        main_menu_handler, draw_handler, "choicebox", MAIN_MENU_SIZE, 1, main_canvas);
    if(!choicebox) {
        printf("no choicebox\n");
        return;
    }

    evas_object_event_callback_add(choicebox,
                                   EVAS_CALLBACK_KEY_UP,
                                   &main_win_key_handler,
                                   main_canvas);

    ecore_evas_callback_resize_set(main_win, main_win_resize_handler);
    ecore_evas_callback_focus_in_set(main_win, main_win_focus_in_handler);

    evas_object_show(main_canvas_edje);
    ecore_evas_show(main_win);

    ecore_main_loop_begin();
}

static
void exit_all(void* param __attribute__((unused))) {
    ecore_main_loop_quit();
}

int main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
    setlocale(LC_ALL, "");
    textdomain("gm");
    if(!init_langs())
        die("Unable to init langs\n");
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

    run();

    if(_gm_keys)
        keys_free(_gm_keys);

    gm_socket_server_stop();

    /* Keep valgrind happy */
    edje_file_cache_set(0);
    edje_collection_cache_set(0);

    ecore_con_shutdown();
    efreet_shutdown();
    edje_shutdown();
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
    shutdown_langs();
    return 0;
}
