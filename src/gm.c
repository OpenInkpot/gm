#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libintl.h>
#define _(x) x

#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <Efreet.h>
#include <Ecore_Con.h>

#include <echoicebox.h>
#include "apps.h"
#include "setup.h"
#include "lang.h"
#include "sock.h"
#include "choices.h"
#include "raise.h"
#include "clock.h"

struct main_menu_item {
    char *title;
    void (*execute)(Evas *evas_ptr, void *arg);
    void *argument;
    char * icon_signal;
};

void run_subshell(Evas * e __attribute__((unused)),
                  void * arg) {
    Ecore_Exe *exe;
    printf("Run subshell\n");
    exe = ecore_exe_run((const char *) arg, NULL);
    if(exe)
        ecore_exe_free(exe);
};

void stub(Evas * e __attribute__((unused)), void * arg) {
    if(!arg)
        arg="<none>";
    printf("Stub %s\n", (char *)arg);
};

struct main_menu_item main_menu[] = {
        {_("Current book"), raise_fbreader, NULL, "set-icon-none" }, // Special
        {_("Library"), run_subshell, "/usr/bin/madshelf --filter=books", "set-icon-lib" },
        {_("Images"), run_subshell, "/usr/bin/madshelf --filter=images",
                    "set-icon-photo"},
        /* {_("Audio"), stub, "Audio", "set-icon-phono"}, */
        {" ", stub, "", NULL},
        {" ", stub, "", NULL},
        {_("Applications"), &run_applications, "Applications", "set-icon-apps"},
        {_("Games"), &run_applications , "Games", "set-icon-games"},
        {_("Setup"), &settings_menu, "Setup", "set-icon-setup"},
        {_("Clock setup"), &run_subshell , "/usr/bin/etimetool", "set-icon-clock"},
        {NULL, NULL, NULL, NULL},
};

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
    {
        printf("Invalidate\n");
        choicebox_invalidate_item(choicebox, 0);
    }
}

static void draw_handler(Evas_Object* choicebox,
                         Evas_Object* item,
                         int item_num,
                         int page_position,
                         void* param)
{
    /* All time formatting taken from libc manual, don't hurt me */
/**    char buf[256];
    time_t curtime;
    struct tm *loctime; */
    struct bookinfo_t *bookinfo;

    if(main_menu[item_num].icon_signal)
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
            edje_object_part_text_set(item, "leftbottom",bookinfo->series);
        } else {
            edje_object_part_text_set(item, "text",
                gettext("<inactive>No book is open</inactive>"));
        }
        gm_free_titles(bookinfo);
    } else
/*    if ((item_num == 8) && main_menu[item_num].title) {
        curtime = time (NULL);
        loctime = localtime (&curtime);
        strftime(buf, 256, gettext(main_menu[item_num].title), loctime);
        edje_object_part_text_set(item, "text", buf);
    } else */
    if (main_menu[item_num].title) {
        edje_object_part_text_set(item, "text",
        gettext(main_menu[item_num].title));
    }

   fprintf(stderr, "handle: choicebox: %p, item: %p, item_num: %d, page_position: %d, param: %p\n",
          choicebox, item, item_num, page_position, param);
}



static void handler(Evas_Object* choicebox,
                    int item_num,
                    bool is_alt,
                    void* param)
{
   printf("handle: choicebox: %p, item_num: %d, is_alt: %d, param: %p\n",
          choicebox, item_num, is_alt, param);
   main_menu[item_num].execute(param, main_menu[item_num].argument);
}


static void main_win_resize_handler(Ecore_Evas* main_win)
{
   Evas* canvas = ecore_evas_get(main_win);
   int w, h;
   evas_output_size_get(canvas, &w, &h);

   Evas_Object* main_canvas_edje = evas_object_name_find(canvas, "main_canvas_edje");
   evas_object_resize(main_canvas_edje, w, h);
}


static void main_win_signal_handler(void* param,
        Evas_Object* o __attribute__((unused)),
        const char* emission, const char* source)
{
    Evas_Object* r = evas_object_name_find((Evas *) param, "choicebox");
    evas_object_del(r);
    printf("%s -> %s\n", source, emission);
}

static void main_win_key_handler(void* param __attribute__((unused)),
        Evas* e __attribute__((unused)),
        Evas_Object *r __attribute__((unused)), void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    fprintf(stderr, "kn: %s, k: %s, s: %s, c: %s\n", ev->keyname, ev->key, ev->string, ev->compose);
//    if(!strcmp(ev->keyname, "Escape"))
//       ecore_main_loop_quit();
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
   gm_init_clock_and_battery(main_canvas_edje);
   edje_object_signal_callback_add(main_canvas_edje, "*", "*", main_win_signal_handler, NULL);
   edje_object_part_text_set(main_canvas_edje, "footer", "");
   edje_object_part_text_set(main_canvas_edje, "path", "");
   evas_object_move(main_canvas_edje, 0, 0);
   evas_object_resize(main_canvas_edje, 600, 800);


   Evas_Object* choicebox = choicebox_push(NULL, main_canvas,
        handler, draw_handler, "choicebox", 9, main_canvas);
   if(!choicebox) {
        printf("no echoicebox\n");
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
   gm_socket_server_stop();

   ecore_con_shutdown();
   efreet_shutdown();
   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   evas_shutdown();
   shutdown_langs();
   return 0;
}
