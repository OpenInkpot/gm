#define _GNU_SOURCE
#include <stdio.h>
#include <Evas.h>
#include <Edje.h>
#include <echoicebox.h>
#include "setup.h"
#include "lang.h"
#include "choices.h"
#include "sound_control.h"
#include "screen_update_control.h"


struct setup_menu_item_t {
    const char *format;
    void (*draw)(Evas_Object *self);
    void (*select) (Evas_Object *self);
    void *arg;
};


const char * screen_states[] = {
    "N/A",
    "Full",
    "Adaptive",
    "Partial"
};

static void
screen_draw(Evas_Object *item)
{
    char *buf;
    screen_update_t scr = detect_screen_update_type();
    asprintf(&buf, "Screen update : %s", screen_states[scr+1]);
    edje_object_part_text_set(item, "text", buf);
    free(buf);
}

static void
screen_set(Evas_Object * self) {
    screen_update_t scr = detect_screen_update_type();
    if(scr < 0)
        return;

    scr ++;
    if(scr > SCREEN_UPDATE_PARTIAL)
        scr = SCREEN_UPDATE_FULL;
    printf("screen: %d\n", scr);
        set_screen_update_type(scr);
        choicebox_invalidate_item(self, 1);
}


const char * sound_states[] = {
    "N/A",
    "OFF",
    "ON"
};

static void
sound_draw(Evas_Object *item)
{
    char *buf;
    sound_t snd = detect_sound();
    asprintf(&buf, "Sound : %s", sound_states[snd+1]);
    edje_object_part_text_set(item, "text", buf);
    free(buf);
}

static void
sound_set(Evas_Object * self) {
    sound_t snd = detect_sound();
    if(snd == SOUND_ON)
       set_sound(SOUND_OFF);
    else if (snd == SOUND_OFF)
       set_sound(SOUND_OFF);
    choicebox_invalidate_item(self, 2);
}

static void
language_draw(Evas_Object *item)
{
    char *buf;
    asprintf(&buf, "Language : %s", current_lang());
    edje_object_part_text_set(item, "text", buf);
    free(buf);
}

#define MENU_ITEMS_NUM 3
struct setup_menu_item_t setup_menu_items[] = {
    {"Screen update: %s", &screen_draw, &screen_set, 0},
    {"Sound: %s",  &sound_draw, &sound_set, 0},
    {"Language: %s", &language_draw, &lang_menu, 0},
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
