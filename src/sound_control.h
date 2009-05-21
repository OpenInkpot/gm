#ifndef SOUND_CONTROL_H
#define SOUND_CONTROL_H

typedef enum
{
    SOUND_NOT_AVAILABLE = -1,
    SOUND_OFF,
    SOUND_ON
} sound_t;

sound_t detect_sound();
int set_sound(sound_t);

#endif


