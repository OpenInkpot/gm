#include "sound_control.h"

#include <errno.h>

/* FIXME */
sound_t detect_sound()
{
    return SOUND_NOT_AVAILABLE;
}

/* FIXME */
int set_sound(sound_t sound __attribute__((unused)))
{
    errno = ENOSYS;
    return -1;
}
