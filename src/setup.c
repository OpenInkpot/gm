#define _GNU_SOURCE
#include <dlfcn.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
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
#include <libeoi_utils.h>
#include "setup.h"
#include "graph.h"
#include "choices.h"
#include "gm-configlet.h"

static Efreet_Ini *_settings;
static char *_settings_path;

static void
gm_settings_save()
{
    efreet_ini_save(_settings, _settings_path);
}


static void
main_view_draw(void *data __attribute__((unused)), Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Main menu view"));
    edje_object_part_text_set(item, "value",
        gm_graphics_mode_get() ? gettext("Graphic") : gettext("Text"));

    edje_object_signal_emit(item, gm_graphics_mode_get()
                            ? "set-icon-main-menu-graphical"
                            : "set-icon-main-menu-text", "");
}

static void
main_view_set(void *data __attribute__((unused)),
                Evas_Object *item __attribute__((unused)))
{
    bool value = (gm_graphics_mode_get() ? 0 : 1);
    gm_graphics_mode_set(value);
    efreet_ini_boolean_set(_settings, "graphics", value);
    gm_settings_save();
}

/* not API, private structure */
typedef struct configlet_t  configlet_t;
struct configlet_t {
    void *module;
    const configlet_plugin_t *methods;
    void *instance;
    const char *sort_key;
};

Evas_Object *
gm_configlet_submenu_push(Evas_Object *parent,
                    void (*select)(Evas_Object *, int, bool, void*),
                    void (*draw)(Evas_Object*, Evas_Object *, int, int, void*),
                    int items, void *param)
{
    Evas *canvas = evas_object_evas_get(parent);
    return choicebox_push(parent, canvas, select, draw, "configlet-submenu",
            items, CHOICEBOX_GM_SETTINGS, param);
}

/* de-facto alias for choicebox_pop, but documented as configlet API */
void
gm_configlet_submenu_pop(Evas_Object *submenu)
{
    choicebox_pop(submenu);
}


static int
sort_cb(const void *left, const void *right)
{
    if(!left) return 1;
    if(!right) return -1;
    return strcmp(((configlet_t *)left)->sort_key,
                  ((configlet_t *)right)->sort_key);
}

#define CONFIGLETS_DIR  "/usr/lib/gm/configlets"

static const char *
get_configlets_dir()
{
    return getenv("CONFIGLETS_DIR")
        ? getenv("CONFIGLETS_DIR") :
        CONFIGLETS_DIR;
}

static int filter_files(const struct dirent* d)
{
    unsigned short int len = _D_EXACT_NAMLEN(d);
    return (len > 2) && !strcmp(d->d_name + len - 3, ".so");
}

static configlet_t *
make_configlet(const configlet_plugin_t *methods, void *module)
{
    configlet_t *configlet=calloc(1, sizeof(configlet_t));
    if(!configlet)
        err(1, "Out of memory while loading configlet\n");

    configlet->methods = methods;
    if(configlet->methods->load)
        configlet->instance = configlet->methods->load();

    configlet->module = module; /* save, to unload later */

    /* ugly, but compatible with builtins */
    configlet->sort_key = configlet->methods->sort_key;

    return configlet;
}

static configlet_t *
load_single_plugin(char *name)
{
    char *libname = xasprintf("%s/%s", get_configlets_dir(), name);
    if(!libname)
        err(1, "Out of memory while load configlet %s\n", name);

    void *libhandle = dlopen(libname, RTLD_NOW | RTLD_LOCAL);
    if(!libhandle)
    {
        fprintf(stderr, "unable to load %s: %s\n", libname, dlerror());
        free(libname);
        return NULL;
    };

    /* Remove '.so' from filename */
    name[strlen(name)-3] = 0;

    char *configlet_name = xasprintf("configlet_%s", name);
    if(!configlet_name)
        err(1, "Out of memory while load configlet %s\n", name);

    configlet_constructor_t ctor =
        dlsym(libhandle, configlet_name);

    if(!ctor)
    {
        fprintf(stderr, "Unable to get entry point in %s: %s", name, dlerror());
        free(configlet_name);
        free(libname);
        dlclose(libhandle);
        return NULL;
    }

    free(configlet_name);
    free(libname);
    const configlet_plugin_t *methods =  ctor();
    return make_configlet(methods, libhandle);
}




Eina_List *
settings_menu_load_plugins()
{
    Eina_List *lst = NULL;
    int i;
    struct dirent **files;
    int nfiles = scandir(get_configlets_dir(),
            &files, &filter_files,  &versionsort);

    if(nfiles == -1)
    {
        fprintf(stderr, "Unable to load configlets from %s: %s\n",
            get_configlets_dir(), strerror(errno));
        return NULL;
    }

    for(i = 0; i != nfiles; ++i)
    {
        configlet_t *configlet = load_single_plugin(files[i]->d_name);
        if(configlet)
            lst = eina_list_append(lst, configlet);
    }
    return lst;
}

static void
setup_builtins(Eina_List **lst)
{
    static const configlet_plugin_t main_view_builtin = {
        .load = NULL,
        .unload = NULL,
        .draw =  &main_view_draw,
        .select = &main_view_set,
        .sort_key = "05main-menu-view",
    };

    *lst = eina_list_append(*lst,
        make_configlet(&main_view_builtin, NULL));
};


Eina_List *
settings_menu_load()
{
    Eina_List *lst = settings_menu_load_plugins();
    setup_builtins(&lst);
    lst = eina_list_sort(lst, eina_list_count(lst), sort_cb);
    return lst;
}

static void
_settings_menu_unload(void *data,
    Evas *canvas __attribute__((unused)),
    Evas_Object *object __attribute__((unused)),
    void * event_type __attribute__((unused)))
{
    configlet_t *configlet;
    Eina_List *list = data;
    EINA_LIST_FREE(list, configlet)
    {
        if(configlet->methods && configlet->instance)
            configlet->methods->unload(configlet->instance);
        if(configlet->module)
            dlclose(configlet->module);
        free(configlet);
    }
}

static void settings_draw(Evas_Object *choicebox,
                         Evas_Object *item,
                         int item_num,
                         int page_position __attribute__((unused)),
                         void *param __attribute__((unused)))
{
    edje_object_signal_emit(item, "set-icon-none", "");
    Eina_List *menu = evas_object_data_get(choicebox, "setup-menu-items");
    configlet_t *configlet = eina_list_nth(menu, item_num);

    configlet->methods->draw(configlet->instance, item);
}

static void settings_handler(Evas_Object *choicebox,
                    int item_num,
                    bool is_alt __attribute__((unused)),
                    void* param __attribute__((unused)))
{
    Eina_List *menu = evas_object_data_get(choicebox, "setup-menu-items");
    configlet_t *configlet = eina_list_nth(menu, item_num);

    configlet->methods->select(configlet->instance, choicebox);
    choicebox_invalidate_item(choicebox, item_num);
}

void settings_menu(Evas *canvas) {
    Evas_Object *choicebox = evas_object_name_find(canvas, "choicebox");
    Eina_List *menu = settings_menu_load();
    choicebox = choicebox_push(choicebox, canvas,
               settings_handler,
               settings_draw,
               "settings-choicebox",
               eina_list_count(menu),
               CHOICEBOX_GM_SETTINGS, NULL);
    if(!choicebox)
        printf("We all dead\n");
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas,
        "main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "title", gettext("Settings"));
    evas_object_event_callback_add(choicebox, EVAS_CALLBACK_DEL,
        &_settings_menu_unload, menu);
    evas_object_data_set(choicebox, "setup-menu-items", menu);
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
