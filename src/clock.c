#include <time.h>
#include <stdio.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore.h>

#include <libeoi_clock.h>
#include <libeoi_battery.h>

#include "clock.h"
#include "graph.h"


static int
update_clock_gr(void *ptr)
{
    Evas *evas = (Evas *) ptr;
    gm_graphics_show_clock(evas);
    return 1;
}

void
gm_init_clock_and_battery(Evas_Object *top, Evas *evas) {
    ecore_timer_add(60, &update_clock_gr, evas);
    eoi_run_clock(top);
    eoi_run_battery(top);
}
