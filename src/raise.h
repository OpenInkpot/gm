#ifndef RAISE_H
#define RAISE_H

#include <Evas.h>
#include <Ecore_X.h>

struct bookinfo_t {
    char *author;
    char *title;
    char *filename;
    char *filepath;
    char *series;
    int series_number;
    char *type;
    char *size;
    int current_position;
    int pages_count;
};

extern struct bookinfo_t *
gm_get_titles();

extern void
gm_free_titles(struct bookinfo_t *titles);

extern void
raise_fbreader(Evas * e);

Ecore_X_Window gm_get_active_document_window();

#endif
