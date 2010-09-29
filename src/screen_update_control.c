#include <liblops.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gm-configlet.h>
#include "screen_update_control.h"

#define _(x) x

/* FIXME: HAL? N516-only right now */
#define SCREEN_UPDATE_CONTROL_FILE "/sys/class/graphics/fb1/manual_refresh_threshold"

#define BUFSIZE 16

static int detect_threshold(const char *control_file)
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

    if(threshold < 0 || threshold > 100)
    {
        errno = ERANGE;
        return -1;
    }

    return threshold;
}

static int set_threshold(const char *control_file, int threshold)
{
    int fd = open(control_file, O_WRONLY | O_TRUNC);
    if(fd == -1)
        return -1;

    char buf[BUFSIZE];
    snprintf(buf, BUFSIZE, "%d", threshold);

    if(-1 == writen(fd, buf, strlen(buf)))
    {
        close(fd);
        return -1;
    }

    if(-1 == close(fd))
        return -1;

    return 0;
}

/* Policy */

static screen_update_t
detect_screen_update_type()
{
    int val = detect_threshold(SCREEN_UPDATE_CONTROL_FILE);
    if(val == -1)
        return SCREEN_UPDATE_NOT_AVAILABLE;
    if(val <= 5)
        return SCREEN_UPDATE_FULL;
    if(val < 95)
        return SCREEN_UPDATE_ADAPTIVE;
    return SCREEN_UPDATE_PARTIAL;
}

static int
set_screen_update_type(screen_update_t screen_update)
{
    int val;
    switch(screen_update)
    {
    case SCREEN_UPDATE_FULL:
        val = 5;
        break;
    case SCREEN_UPDATE_ADAPTIVE:
        val = 90;
        break;
    case SCREEN_UPDATE_PARTIAL:
        val = 100;
        break;
    default:
        errno = EINVAL;
        return -1;
    }

    return set_threshold(SCREEN_UPDATE_CONTROL_FILE, val);
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
screen_draw(void *data __attribute__((unused)), Evas_Object *item)
{
    screen_update_t scr = detect_screen_update_type();
    edje_object_part_text_set(item, "title", gettext("Screen update"));
    edje_object_part_text_set(item, "value", gettext(screen_states[scr+1]));
    edje_object_signal_emit(item, screen_state_icons[scr+1], "");
}

static void
screen_set(void *data, Evas_Object *self) {
    screen_update_t scr = detect_screen_update_type();
    if(scr < 0)
        return;

    scr++;
    if(scr > SCREEN_UPDATE_PARTIAL)
        scr = SCREEN_UPDATE_FULL;
    set_screen_update_type(scr);
    gm_configlet_invalidate_parent(self, data);
}

const configlet_plugin_t *
configlet_screen(void)
{
    static const configlet_plugin_t configlet = {
        .load = NULL,
        .unload = NULL,
        .draw = screen_draw,
        .select = screen_set,
        .sort_key = "01screen",
    };
    return &configlet;
}

