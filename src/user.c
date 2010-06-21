/*
 * gm - GUI "global menu" application.
 *
 * Copyright © 2010 Mikhail Gusarov <dottedmag@dottedmag.net>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "user.h"
#include <libintl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <libchoicebox.h>
#include "gm-configlet.h"

#define STATE_DIR LOCALSTATEDIR "/lib/gm"

const char *
get_user_name()
{
    if (!getenv("USER"))
        return "user";
    if (!strcmp(getenv("USER"), "root"))
        return "user";
    return getenv("USER");
}

static void
store_cur_user_file(const char *name, const char *home_prefix)
{
    FILE *f = fopen(STATE_DIR "/cur-user", "w");
    fprintf(f, "export USER=%s\nexport HOME=%s%s\n", name,
            home_prefix, name);
    fclose(f);
}

void
set_user(const char *name)
{
    if (!strcmp(name, "user"))
        name = "root";

    store_cur_user_file(name, !strcmp(name, "root") ? "/" : "/home/" );
}

/* Users. */

/* FIXME: All of this is a gross hack. It could me much more useful to use real
 * separate accounts, and not just separate home dirs as now. */

#define NUM_USERS 5

static void
user_set_handler(Evas_Object *choicebox __attribute__((unused)),
                 int item_num,
                 bool is_alt __attribute__((unused)),
                 void *param __attribute__((unused)))
{
    char user[8] = "user";
    if (item_num != 0)
        sprintf(user, "user%d", item_num);
    set_user(user);
    ecore_main_loop_quit();
}

static void
user_set_draw(Evas_Object *choicebox __attribute__((unused)),
              Evas_Object *item, int item_num,
              int page_position __attribute__((unused)),
              void *param __attribute__((unused)))
{
    char user[8] = "user";
    if (item_num != 0)
        sprintf(user, "user%d", item_num);
    char homedir[20];
    sprintf(homedir, "/home/%s", user);
    bool user_exists = ecore_file_exists(homedir);

    char text[40];
    sprintf(text, "%s%s%s", user_exists ? "" : "<inactive>",
            gettext(user), user_exists ? "" : "</inactive>");


    edje_object_part_text_set(item, "title", text);
}

static void
user_set_main_menu(void *data __attribute__((unused)), Evas_Object *item)
{
    Evas *canvas = evas_object_evas_get(item);
    Evas_Object *choicebox;
    choicebox = gm_configlet_submenu_push(item,
                               user_set_handler,
                               user_set_draw,
                               NUM_USERS,
                               NULL);
    Evas_Object *main_canvas_edje = evas_object_name_find(canvas,"main_canvas_edje");
    edje_object_part_text_set(main_canvas_edje, "title", gettext("Profile"));

    int curidx = 0;
    const char *username = get_user_name();
    sscanf(username, "user%d", &curidx);
    choicebox_set_selection(choicebox, curidx);
}

static void
user_draw_main_menu(void *data __attribute__((unused)), Evas_Object *item)
{
    edje_object_part_text_set(item, "title", gettext("Profile"));
    edje_object_part_text_set(item, "value", gettext(get_user_name()));
    edje_object_signal_emit(item, "set-icon-users", "");
}


const configlet_plugin_t *
configlet_user(void)
{
    static const configlet_plugin_t configlet = {
        .load = NULL,
        .unload = NULL,
        .draw = user_draw_main_menu,
        .select = user_set_main_menu,
        .sort_key = "06user",
    };
    return &configlet;
}
