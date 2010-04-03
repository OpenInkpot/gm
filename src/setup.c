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
#include "user.h"

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
    static char *version = NULL;
    if (!version) {
        version = "N/A";

        int fd = open("/etc/openinkpot-version", O_RDONLY);
        if (fd != -1) {
            char version_str[VERSION_SIZE];
            int r = readn(fd, version_str, VERSION_SIZE-1);
            if (r > 0) {
                version_str[r-1] = '\0';
                char *c = strchr(version_str,'\n');
                if(c)
                    *c = '\0';
                version = strdup(version_str);
            }
            close(fd);
        }
    }

    edje_object_part_text_set(item, "title", gettext("Version"));
    edje_object_part_text_set(item, "value", version);
    edje_object_signal_emit(item, "set-icon-version", "");
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

const char *screen_state_icons[] = {
    "set-icon-none",
    "set-icon-update-full",
    "set-icon-update-adaptive",
    "set-icon-update-zone",
};

static void
screen_draw(Evas_Object *item)
{
    screen_update_t scr = detect_screen_update_type();
    edje_object_part_text_set(item, "title", gettext("Screen update"));
    edje_object_part_text_set(item, "value", gettext(screen_states[scr+1]));
    edje_object_signal_emit(item, screen_state_icons[scr+1], "");
}

static void
screen_set(Evas_Object *self) {
    screen_update_t scr = detect_screen_update_type();
    if(scr < 0)
        return;

    scr++;
    if(scr > SCREEN_UPDATE_PARTIAL)
        scr = SCREEN_UPDATE_FULL;
        set_screen_update_type(scr);
        choicebox_invalidate_item(self, 0);
}

static void
rotation_draw(Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Screen rotation type"));
    edje_object_part_text_set(item, "value", gm_current_rotation());
    gm_set_rotation_icon(item);
}

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
    edje_object_signal_emit(item, "set-icon-lang", "");
}

static void
datetime_draw(Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Clock setup"));
    edje_object_part_text_set(item, "value", "");
    edje_object_signal_emit(item, "set-icon-time", "");
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

    edje_object_signal_emit(item, gm_graphics_mode_get()
                            ? "set-icon-main-menu-graphical"
                            : "set-icon-main-menu-text", "");
}

static void
main_view_set(Evas_Object *item __attribute__((unused)))
{
    bool value = (gm_graphics_mode_get() ? 0 : 1);
    gm_graphics_mode_set(value);
    efreet_ini_boolean_set(_settings, "graphics", value);
    gm_settings_save();
}

/* Users. */

/* FIXME: All of this is a gross hack. It could me much more useful to use real
 * separate accounts, and not just separate home dirs as now. */

#define NUM_USERS 5

static void
user_draw_main_menu(Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Profile"));
    edje_object_part_text_set(item, "value", gettext(get_user_name()));
    edje_object_signal_emit(item, "set-icon-users", "");
}

static void
user_set_handler(Evas_Object *choicebox __attribute__((unused)),
                 int item_num,
                 bool is_alt __attribute__((unused)),
                 void *param __attribute__((unused)))
{
    char user[8] = "user";
    if (item_num != 0)
        sprintf(user, "user%d", item_num);
    set_user(user);
    ecore_main_loop_quit();
}

static void
user_set_draw(Evas_Object *choicebox __attribute__((unused)),
              Evas_Object *item, int item_num,
              int page_position __attribute__((unused)),
              void *param __attribute__((unused)))
{
    char user[8] = "user";
    if (item_num != 0)
        sprintf(user, "user%d", item_num);
    char homedir[20];
    sprintf(homedir, "/home/%s", user);
    bool user_exists = ecore_file_exists(homedir);

    char text[40];
    sprintf(text, "%s%s%s", user_exists ? "" : "<inactive>",
            gettext(user), user_exists ? "" : "</inactive>");

    edje_object_part_text_set(item, "title", text);
}

static void
user_set_main_menu(Evas_Object *item)
{
    Evas *canvas = evas_object_evas_get(item);
    Evas_Object *choicebox;
    choicebox = choicebox_push(item, canvas,
                               user_set_handler,
                               user_set_draw,
                               "user-choicebox", NUM_USERS,
                               CHOICEBOX_GM_SETTINGS, NULL);
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "title", gettext("Profile"));

    int curidx = 0;
    const char *username = get_user_name();
    sscanf(username, "user%d", &curidx);
    choicebox_set_selection(choicebox, curidx);
}

struct setup_menu_item_t setup_menu_items[] = {
    {&screen_draw, &screen_set, 0},
    {&rotation_draw, &gm_rotation_menu, 0},
    {&language_draw, &lang_menu, 0},
    {&datetime_draw, &datetime_set, 0},
    {&main_view_draw, main_view_set, 0},
    {&user_draw_main_menu, user_set_main_menu, 0},
    {&version_draw, &version_set, 0},
};

#define MENU_ITEMS_NUM (sizeof(setup_menu_items)/sizeof(setup_menu_items[0]))

static void settings_draw(Evas_Object *choicebox __attribute__((unused)),
                         Evas_Object *item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void *param __attribute__((unused)))
{
    edje_object_signal_emit(item, "set-icon-none", "");
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
               "settings-choicebox", MENU_ITEMS_NUM, CHOICEBOX_GM_SETTINGS, NULL);
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
