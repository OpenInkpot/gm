#include <stdbool.h>

#include "raise.h"
#include "apps_cleanup.h"

/* Crazy hack: need to be replaced with something saner */
void gm_apps_cleanup(Ecore_Evas* win)
{
    if(!getenv("GM_APPS_CLEANUP_ENABLE"))
        return;

    Ecore_X_Window skip[] = {
        ecore_evas_software_x11_window_get(win),
        gm_get_active_document_window(),
        (unsigned)-1
    };

    Ecore_X_Window root = ecore_x_window_root_first_get();

    ecore_x_query_tree_prefetch(root);
    ecore_x_query_tree_fetch();
    int num;
    Ecore_X_Window* windows = ecore_x_window_children_get(root, &num);

    for(int i = 0; i < num; ++i)
    {
        ecore_x_get_window_attributes_prefetch(windows[i]);
        ecore_x_get_window_attributes_fetch();

        Ecore_X_Window_Attributes at;
        ecore_x_window_attributes_get(windows[i], &at);

        if(at.visible)
        {
            bool del = true;
            for(int j = 0; skip[j] != (unsigned)-1; ++j)
                if(windows[i] == skip[j])
                    del = false;
            if(del)
                ecore_x_window_delete_request_send(windows[i]);
        }
    }

    free(windows);
}

