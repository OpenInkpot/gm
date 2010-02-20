#include <libintl.h>
#include <libeoi.h>
#include <libeoi_help.h>
#include "gm.h"
#include "help.h"

void gm_help_show(Evas* evas)
{
    eoi_help_show(evas, "gm", NULL, gettext("GM: Help"),
        gm_keys(), NULL);
}
