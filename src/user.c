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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
