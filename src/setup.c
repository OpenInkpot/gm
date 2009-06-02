#define _GNU_SOURCE
#include <stdio.h>
#include <Evas.h>
#include <Edje.h>
#include <echoicebox.h>
#include "setup.h"
#include "lang.h"
#include "choices.h"


struct setup_menu_item_t {
    const char *format;
    void (*draw)(Evas_Object *self);
    void (*select) (Evas_Object *self);
    void *arg;
};


static void
screen_draw(Evas_Object *item)
{
    edje_object_part_text_set(item, "text", "Screen update : ON");
}

static void
screen_set(Evas_Object * self __attribute__((unused))) {
}

static void
sound_draw(Evas_Object *item)
{
    edje_object_part_text_set(item, "text", "Sound : ON");
}

static void
sound_set(Evas_Object * self __attribute__((unused))) {
}

static void
language_draw(Evas_Object *item)
{
    char *buf;
    asprintf(&buf, "Language : %s", current_lang());
    edje_object_part_text_set(item, "text", buf);
    free(buf);
}

static void
language_set(Evas_Object * self) {
   lang_menu(self);
}


#define MENU_ITEMS_NUM 3
struct setup_menu_item_t setup_menu_items[] = {
    {"Screen update: %s", &screen_draw, &screen_set, 0},
    {"Sound: %s",  &sound_draw, &sound_set, 0},
    {"Language: %s", &language_draw, &language_set, 0},
    {NULL, NULL, NULL, NULL},
};

static void settings_draw(Evas_Object* choicebox __attribute__((unused)),
                         Evas_Object* item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void* param __attribute__((unused)))
{
    setup_menu_items[item_num].draw(item);
}

static void settings_handler(Evas_Object* choicebox,
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param __attribute__((unused)))
{
    setup_menu_items[item_num].select(choicebox);
}

void settings_menu(Evas *canvas, void * arg __attribute__((unused))) {
    Evas_Object * choicebox = evas_object_name_find(canvas, "choicebox");
    choicebox = choicebox_push(choicebox, canvas,
               settings_handler,
               settings_draw,
               "settings-choicebox", MENU_ITEMS_NUM, NULL);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object * main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "contents", "Settings");
}
