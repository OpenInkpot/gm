#ifndef _GM_CONFIGLET
#define _GM_CONFIGLET

#include <stdbool.h>
#include <Evas.h>

/* API */
typedef struct configlet_plugin_t configlet_plugin_t;
struct configlet_plugin_t {
    void * (*load)(void);
    int (*draw)(void *, Evas_Object *);
    int (*select)(void*, Evas_Object *);
    int (*unload)(void *);
    const char *sort_key; // sort key -- sttically allocated
};

Evas_Object *
gm_configlet_submenu(Evas_Object *parent,
                    void (*select)(Evas_Object *, int, bool, void*),
                    void (*draw)(Evas_Object*, Evas_Object *, int, int, void*),
                    int items);

#endif
