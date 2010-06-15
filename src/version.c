#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <liblops.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore.h>
#include "gm-configlet.h"

#define VERSION_SIZE 1024

static void *
version_load(void)
{
    char *version = NULL;

    int fd = open("/etc/openinkpot-version", O_RDONLY);
    if (fd != -1) {
        char version_str[VERSION_SIZE];
        int r = readn(fd, version_str, VERSION_SIZE-1);
        if (r > 0) {
            version_str[r-1] = '\0';
            char *c = strchr(version_str,'\n');
            if(c)
                *c = '\0';
            version = strdup(version_str);
        }
        close(fd);
    }

    if (!version)
        version = strdup("N/A");
    return version;
}

static void
version_draw(void *data, Evas_Object *item)
{
    char *version = (char *) data;
    edje_object_part_text_set(item, "title", gettext("Version"));
    edje_object_part_text_set(item, "value", version);
    edje_object_signal_emit(item, "set-icon-version", "");
}

static void
version_set(void *data __attribute__((unused)),
            Evas_Object *item __attribute__((unused)))
{
    Ecore_Exe *exe = ecore_exe_run("/usr/bin/eabout", NULL);
    if(exe)
        ecore_exe_free(exe);
}

static void
version_unload(void *data)
{
    free(data);
}

const configlet_plugin_t *
configlet_version(void)
{
    static const configlet_plugin_t configlet = {
        .load = version_load,
        .unload = version_unload,
        .draw = version_draw,
        .select = version_set,
        .sort_key = "99version",
    };
    return &configlet;
}
