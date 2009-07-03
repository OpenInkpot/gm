#include <unistd.h>
#include <stdio.h>

#include <Edje.h>

#include "clock.h"

typedef struct
{
    char* now;
    char* min;
    char* max;
} battery_info_t;

static battery_info_t batteries[] =
{
    {
        "/sys/class/power_supply/n516-battery/charge_now",
        "/sys/class/power_supply/n516-battery/charge_empty_design",
        "/sys/class/power_supply/n516-battery/charge_full_design",
    },
    {
        "/sys/class/power_supply/lbookv3_battery/charge_now",
        "/sys/class/power_supply/lbookv3_battery/charge_empty_design",
        "/sys/class/power_supply/lbookv3_battery/charge_full_design",
    },
};

static int _find_battery()
{
    unsigned int i;
    for(i = 0; i < sizeof(batteries)/sizeof(batteries[0]); ++i)
    {
        if(!access(batteries[i].now, R_OK))
            return i;
    }
    return -1;
}

static int _read_int_file(const char* filename)
{
    int res = 0;
    FILE* f = fopen(filename, "r");
    fscanf(f, "%d", &res);
    fclose(f);
    return res;
}

/*
 * -1 - unknown
 * 0..100 - charge
 */
static int _get_state()
{
    int batt = _find_battery();

    if(batt == -1)
        return -1;

    int now = _read_int_file(batteries[batt].now);
    int min = _read_int_file(batteries[batt].min);
    int max = _read_int_file(batteries[batt].max);

    return 100 * (now - min) / (max - min);
}

void update_battery(Evas_Object* top)
{
    int charge = _get_state();

    if(charge < 5)
        edje_object_signal_emit(top, "set_batt_empty", "");
    else if(charge < 25)
        edje_object_signal_emit(top, "set_batt_1/4", "");
    else if(charge < 50)
        edje_object_signal_emit(top, "set_batt_2/4", "");
    else if(charge < 75)
        edje_object_signal_emit(top, "set_batt_3/4", "");
    else if(charge < 100)
        edje_object_signal_emit(top, "set_batt_4/4", "");
    else
        edje_object_signal_emit(top, "set_batt_full", "");
}

