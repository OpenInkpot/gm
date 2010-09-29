#ifndef _GM_CONFIGLET
#define _GM_CONFIGLET

#include <stdbool.h>
#include <Evas.h>

/* API */
typedef struct configlet_plugin_t configlet_plugin_t;
struct configlet_plugin_t {
    void * (*load)(void);
    void (*draw)(void *, Evas_Object *);
    void (*select)(void*, Evas_Object *);
    void (*unload)(void *);
    const char *sort_key; // sort key -- sttically allocated
};

Evas_Object *
gm_configlet_submenu_push(Evas_Object *parent,
                    void (*select)(Evas_Object *, int, bool, void*),
                    void (*draw)(Evas_Object*, Evas_Object *, int, int, void*),
                    int items,
                    void *param);

void
gm_configlet_submenu_pop(Evas_Object *);

/*
    invalidate item  in parent choicebox (usually called from ->select()(
*/
void
gm_configlet_invalidate_parent(Evas_Object *obj, void *instance);


typedef const configlet_plugin_t * (*configlet_constructor_t)(void);

#endif
