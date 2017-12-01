#ifndef _GM_H
#define _GM_H 1

#include <libkeys.h>

#define THEME_EDJE "gm-scalable"

struct bookinfo_t {
    char *author;
    char *title;
    char *filename;
    char *filepath;
    char *series;
    int series_number;
    char *type;
    char *size;
    int current_page;
    int pages_count;
    char *cover_image;
    int cover_size;
};

/* TODO: implement generic book info fetch */
static inline struct bookinfo_t *gm_get_titles()
{
    return NULL;
}

static inline void gm_free_titles(struct bookinfo_t *titles __attribute__((unused)))
{
}

static inline void open_current_book(Evas * e __attribute__((unused)))
{
}

keys_t* gm_keys();
void gm_choicebox_raise_root(Evas*);

#endif
