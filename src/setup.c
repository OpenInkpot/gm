#define _GNU_SOURCE
#include <fcntl.h>
#include <libintl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define _(x) x
#include <liblops.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore_File.h>
#include <Efreet.h>
#include <Ecore.h>
#include <libchoicebox.h>
#include "setup.h"
#include "graph.h"
#include "lang.h"
#include "choices.h"
#include "sound_control.h"
#include "screen_update_control.h"
#include "rotation.h"
#include "run.h"

static Efreet_Ini *_settings;
static char *_settings_path;

struct setup_menu_item_t {
    void (*draw)(Evas_Object *self);
    void (*select) (Evas_Object *self);
    void *arg;
};

static void
gm_settings_save()
{
    efreet_ini_save(_settings, _settings_path);
}

#define VERSION_SIZE 1024

static void
version_draw(Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Version"));
    char version_str[VERSION_SIZE]="N/A";
    int fd = open("/etc/openinkpot-version", O_RDONLY);
    if (fd != -1) {
        int r = readn(fd, version_str, VERSION_SIZE-1);
        if( r > 0) {
            version_str[r-1]='\0';
            char *c = strchr(version_str,'\n');
            if(c)
                *c = '\0';
        }
        close(fd);
    }
    edje_object_part_text_set(item, "value", version_str);
}

static void
version_set(Evas_Object *item __attribute__((unused)))
{
    Ecore_Exe *exe = ecore_exe_run("/usr/bin/eabout", NULL);
    if(exe)
        ecore_exe_free(exe);
}


const char * screen_states[] = {
    _("<inactive>N/A</inactive>"),
    _("Full"),
    _("Adaptive"),
    _("Partial")
};

static void
screen_draw(Evas_Object *item)
{
    screen_update_t scr = detect_screen_update_type();
    edje_object_part_text_set(item, "title", gettext("Screen update"));
    edje_object_part_text_set(item, "value", gettext(screen_states[scr+1]));
}

static void
screen_set(Evas_Object *self) {
    screen_update_t scr = detect_screen_update_type();
    if(scr < 0)
        return;

    scr ++;
    if(scr > SCREEN_UPDATE_PARTIAL)
        scr = SCREEN_UPDATE_FULL;
    printf("screen: %d\n", scr);
        set_screen_update_type(scr);
        choicebox_invalidate_item(self, 0);
}

static void
rotation_draw(Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Screen rotation type"));
    edje_object_part_text_set(item, "value", current_rotation());
}

#if 0
const char * sound_states[] = {
    _("<invisible>N/A</invisible>"),
    _("OFF"),
    _("ON")
};

static void
sound_draw(Evas_Object *item)
{
    sound_t snd = detect_sound();
    edje_object_part_text_set(item, "lefttop", gettext("Sound"));
    edje_object_part_text_set(item, "rightbottom", gettext(sound_states[snd+1]));
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

#endif

static void
language_draw(Evas_Object *item)
{
    /*
      TRANSLATORS: Please make this menu string two-language: 'Language
      (localized) <inactive>/ Language(in English)</inactive>'. This will allow
      users to reset language if current language is unknown to them translation
      is broken due to some reason, like lack of font).
    */
    edje_object_part_text_set(item, "title", gettext("Language <inactive>/ Language</inactive>"));
    edje_object_part_text_set(item, "value", current_lang());
}

static void
datetime_draw(Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Clock setup"));
    edje_object_part_text_set(item, "value", "");
}

static void
datetime_set(Evas_Object *item)
{
    Evas* canvas = evas_object_evas_get(item);
    gm_run_etimetool(canvas);
}

static void
main_view_draw(Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Main menu view"));
    edje_object_part_text_set(item, "value",
        gm_graphics_mode_get() ? gettext("Graphic") : gettext("Text"));
}

static void
main_view_set(Evas_Object *item)
{
    bool value = (gm_graphics_mode_get() ? 0 : 1);
    gm_graphics_mode_set(value);
    efreet_ini_boolean_set(_settings, "graphics", value);
    gm_settings_save();
    // FIXME: why not redrawn automatically?
//    main_view_draw(item);
}

struct setup_menu_item_t setup_menu_items[] = {
    {&screen_draw, &screen_set, 0},
    {&rotation_draw, &rotation_menu, 0},
//    {&sound_draw, &sound_set, 0},
    {&language_draw, &lang_menu, 0},
    {&datetime_draw, &datetime_set, 0},
    {&main_view_draw, main_view_set, 0},
    {&version_draw, &version_set, 0},
};

#define MENU_ITEMS_NUM (sizeof(setup_menu_items)/sizeof(setup_menu_items[0]))

static void settings_draw(Evas_Object *choicebox __attribute__((unused)),
                         Evas_Object *item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void *param __attribute__((unused)))
{
    setup_menu_items[item_num].draw(item);
}

static void settings_handler(Evas_Object *choicebox,
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param __attribute__((unused)))
{
    setup_menu_items[item_num].select(choicebox);
    choicebox_invalidate_item(choicebox, item_num);
}

void settings_menu(Evas *canvas) {
    Evas_Object *choicebox = evas_object_name_find(canvas, "choicebox");
    choicebox = choicebox_push(choicebox, canvas,
               settings_handler,
               settings_draw,
               "settings-choicebox", MENU_ITEMS_NUM, 0, NULL);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas,
        "main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "title", gettext("Settings"));
}

#define USER_CONFIG_DIR "%s/.e/apps/gm"
#define USER_CONFIG_FILE "gm.ini"

static const char *
gm_settings_get(char *key, const char* default_value)
{
    const char *tmp = efreet_ini_string_get(_settings, key);
    if(!tmp)
        return default_value;
    return tmp;
}

void
gm_settings_load()
{
    char *path;
    char *home = getenv("HOME");
    if(!home)
        home="/home";
    asprintf(&path, USER_CONFIG_DIR, home);
    ecore_file_mkpath(path);
    free(path);

    asprintf(&_settings_path, USER_CONFIG_DIR "/" USER_CONFIG_FILE, home);
    _settings = efreet_ini_new(_settings_path);
    if(!efreet_ini_section_set(_settings, "config"))
    {
        efreet_ini_section_add(_settings, "config");
        efreet_ini_section_set(_settings, "config");
    }

    gm_graphics_mode_set(strcmp(gm_settings_get("graphics", "true"), "false"));
}

void
gm_settings_free()
{
    if(_settings)
        efreet_ini_free(_settings);
    free(_settings_path);
}
