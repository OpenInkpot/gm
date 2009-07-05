#ifndef _RAISE_H
#define _RAISE_H 1

#include <Evas.h>


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

#endif
