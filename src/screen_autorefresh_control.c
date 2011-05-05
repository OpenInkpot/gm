#define _GNU_SOURCE
#include <liblops.h>
#include <libeoi_numentry.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>

#include <Edje.h>

#include <gm-configlet.h>

#define _(x) x

/* FIXME: HAL? N516-only right now */
#define SCREEN_UPDATE_CONTROL_FILE "/sys/class/graphics/fb1/autorefresh_interval"

#define MAX_VALUE 100

#define BUFSIZE 16

static int detect_interval(const char *control_file)
{
    int fd = open(control_file, O_RDONLY);
    if(fd == -1)
        return -1;

    char val[BUFSIZE+1] = ""; /* +1 for terminating zero */
    ssize_t size = readn(fd, val, BUFSIZE);
    if(size == -1)
    {
        close(fd);
        return -1;
    }

    if(close(fd) == -1)
        return -1;

    if(size == 0)
    {
        errno = ENODATA;
        return -1;
    }

    int i;
    for(i = 0; i != size; ++i)
    {
        if(val[i] == '\n')
            break;
        if(!isdigit(val[i]))
        {
            errno = EINVAL;
            return -1;
        }
    }

    int threshold = atoi(val);

    if(threshold < 0)
    {
        errno = ERANGE;
        return -1;
    }

    return threshold;
}

static int set_interval(const char *control_file, int interval)
{
    int fd = open(control_file, O_WRONLY | O_TRUNC);
    if(fd == -1)
        return -1;

    char buf[BUFSIZE];
    snprintf(buf, BUFSIZE, "%d", interval);

    if(-1 == writen(fd, buf, strlen(buf)))
    {
        close(fd);
        return -1;
    }

    if(-1 == close(fd))
        return -1;

    return 0;
}

static void
screen_draw(void *data __attribute__((unused)), Evas_Object *item)
{
    char *buf;
    int val = detect_interval(SCREEN_UPDATE_CONTROL_FILE);

    edje_object_part_text_set(item, "title", gettext("Autorefresh interval"));
    if (val < 0)
        edje_object_part_text_set(item, "value", gettext("N/A"));
    else {
        asprintf(&buf, "%d", val);
        edje_object_part_text_set(item, "value", buf);
        free(buf);
    }
}

struct param {
    void *data;
    Evas_Object *o;
};
typedef struct param param_t;

void entry_handler(Evas_Object *entry __attribute__((unused)), unsigned long value, void *data)
{
    param_t *p = data;

    if (value > MAX_VALUE)
        value = MAX_VALUE;

    set_interval(SCREEN_UPDATE_CONTROL_FILE, value);
    if (p) {
        gm_configlet_invalidate_parent(p->o, p->data);
        free(p);
    }
}


static void
screen_set(void *data, Evas_Object *self) {
    param_t *p = malloc(sizeof(param_t));
    p->data = data;
    p->o = self;
    numentry_new(evas_object_evas_get(self), entry_handler,
            "interval", gettext("Interval"), p);
}

const configlet_plugin_t *
configlet_autorefresh(void)
{
    static const configlet_plugin_t configlet = {
        .load = NULL,
        .unload = NULL,
        .draw = screen_draw,
        .select = screen_set,
        .sort_key = "03autorefresh",
    };
    return &configlet;
}

