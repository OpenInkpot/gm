#include <stdio.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_X_Atoms.h>
#include <xcb/xcb.h>
#include "raise.h"

static
Ecore_X_Atom
gm_get_atom(const char *name)
{
    ecore_x_atom_get_prefetch(name);
    ecore_x_atom_get_fetch();
    return ecore_x_atom_get(name);
}

static
Ecore_X_Window
gm_root_get()
{
    Ecore_X_Screen *s = ecore_x_default_screen_get();
    return  ((xcb_screen_t*)s)->root;
}

static
Ecore_X_Window
gm_get_fbreader()
{
    int rc = 0;
    int num = 0;
    Ecore_X_Atom active_doc_window_id = gm_get_atom("ACTIVE_DOC_WINDOW_ID");
    if(!active_doc_window_id)
    {
        printf("No atom\n");
        return 0;
    }
    printf("Atom: %d\n", active_doc_window_id);
    Ecore_X_Window value;
    Ecore_X_Window root = gm_root_get();
    ecore_x_window_prop_window_get_prefetch(root, active_doc_window_id);
    ecore_x_window_prop_window_get_fetch();
    rc = ecore_x_window_prop_window_get(root, active_doc_window_id,  &value, 1);
    if(rc > 0)
    {
        printf("Got %x, %d\n",value, num);
        return value;
    };
    printf("No value: %d\n", rc);
    return 0;
}

void
raise_fbreader(Evas * e __attribute__((unused)), void * arg __attribute__((unused)))
{
    Ecore_X_Window fbreader = gm_get_fbreader();
    if (fbreader)
    {
        printf("Raising\n");
        ecore_x_window_raise(fbreader);
    }
}

void
get_get_titles(char **titles __attribute__((unused)))
{
}
