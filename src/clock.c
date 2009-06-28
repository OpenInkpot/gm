#include <time.h>
#include <stdio.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore.h>
#include "clock.h"


static int
update_clock(void *ptr) {
    Evas_Object *top = (Evas_Object *) ptr;
    char buf[256];
    time_t curtime;
    struct tm * loctime;
    curtime = time (NULL);
    loctime = localtime (&curtime);
    strftime(buf, 256, "%H:%M", loctime);
    edje_object_part_text_set(top, "clock", buf);
    update_battery(top);
    return 1;
}

void
gm_init_clock_and_battery(Evas_Object *top) {
    ecore_timer_add(60, &update_clock, top);
    update_clock(top);
}
