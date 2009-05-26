#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <Efreet.h>

#include <echoicebox.h>
#include "apps.h"

struct main_menu_item {
    const char *title;
    void (*execute)(void *evas_ptr, void *arg);
    void *argument;
};

void run_subshell(void * e __attribute__((unused)), 
                  void * arg __attribute__((unused))) {
    printf("Run subshell\n");
};

void stub(void * e __attribute__((unused)), void * arg) {
    if(!arg)
        arg="<none>";
    printf("Stub %s\n", (char *)arg);
};

struct main_menu_item main_menu[] = {
    {"Current book: %s", stub, NULL }, // Special
    {"Library", run_subshell, "/usr/bin/madshelf" },
    {"Images", stub, "Images"},
    {"Audio", stub, "Audio"},
    {"", stub, ""},
    {"Applications", stub, "Apps"},
    {"games", &run_applications , "Games"},
    {"Setup", stub, "Setup"},
    {"Clock setup", stub, "Clock"},
    {NULL, NULL, NULL,},
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

static void draw_handler(Evas_Object* choicebox,
                         Evas_Object* item,
                         int item_num,
                         int page_position,
                         void* param)
{
    /* All time formatting taken from libc manual, don't hurt me */
    char buf[256];
    time_t curtime;
    struct tm *loctime;

    if((item_num == 0) && main_menu[item_num].title ) {
        snprintf(buf, 256, main_menu[item_num].title , 
        "Unknown");
        edje_object_part_text_set(item, "choicebox/item/title", buf);
    } else
    if ((item_num == 9) && main_menu[item_num].title) {
        curtime = time (NULL);
        loctime = localtime (&curtime);
        strftime(buf, 256, main_menu[item_num].title, loctime);
        edje_object_part_text_set(item, "choicebox/item/title", buf);
    } else
    if (main_menu[item_num].title) {
        edje_object_part_text_set(item, "choicebox/item/title", 
        main_menu[item_num].title);
    }

   fprintf(stderr, "handle: choicebox: %p, item: %p, item_num: %d, page_position: %d, param: %p\n",
          choicebox, item, item_num, page_position, param);
}


static void page_handler(Evas_Object* choicebox __attribute__((unused)),
                                int a,
                                int b,
                                void* param __attribute__((unused)))
{
   printf("page: %d/%d\n", a, b);
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

   Evas_Object* choicebox = evas_object_name_find(canvas, "choicebox");
   evas_object_resize(choicebox, w, h);
   evas_object_move(choicebox, 0, 0);
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
        Evas* e __attribute__((unused)), Evas_Object *r, void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    fprintf(stderr, "kn: %s, k: %s, s: %s, c: %s\n", ev->keyname, ev->key, ev->string, ev->compose);

    choicebox_aux_key_down_handler(r, ev);

    if(!strcmp(ev->keyname, "Escape"))
       ecore_main_loop_quit();
}

static void run()
{
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);

   Ecore_Evas* main_win = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
   ecore_evas_title_set(main_win, "GM");
   ecore_evas_name_class_set(main_win, "GM", "GM");

   Evas* main_canvas = ecore_evas_get(main_win);

   ecore_evas_callback_delete_request_set(main_win, main_win_close_handler);

   Evas_Object* main_canvas_edje = edje_object_add(main_canvas);
   evas_object_name_set(main_canvas_edje, "main_canvas_edje");
   edje_object_file_set(main_canvas_edje, "/home/dottedmag/openinkpot/gm/themes/gm.edj", "main_window");
   edje_object_signal_callback_add(main_canvas_edje, "*", "*", main_win_signal_handler, NULL);
   evas_object_show(main_canvas_edje);


   Evas_Object* choicebox = choicebox_new(main_canvas, THEME_DIR "/gm.edj",
        "choicebox/item", handler, draw_handler, page_handler, main_canvas);
   choicebox_set_size(choicebox, 16);
   evas_object_name_set(choicebox, "choicebox");
   evas_object_resize(choicebox, 600, 800);
   evas_object_move(choicebox, 0, 0);
   evas_object_show(choicebox);

   evas_object_focus_set(choicebox, true);
   evas_object_event_callback_add(choicebox,
                                  EVAS_CALLBACK_KEY_DOWN,
                                  &main_win_key_handler,
                                  main_canvas);

   ecore_evas_callback_resize_set(main_win, main_win_resize_handler);

   ecore_evas_show(main_win);

   ecore_main_loop_begin();
}

void exit_all(void* param) { 
    ecore_main_loop_quit(); 
}

int main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
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

   ecore_x_io_error_handler_set(exit_all, NULL);
   run();

   efreet_shutdown();
   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   evas_shutdown();
   return 0;
}
