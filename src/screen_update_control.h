#ifndef SCREEN_UPDATE_CONTROL_H
#define SCREEN_UPDATE_CONTROL_H

typedef enum {
    SCREEN_UPDATE_NOT_AVAILABLE = -1,
    SCREEN_UPDATE_FULL,
    SCREEN_UPDATE_ADAPTIVE,
    SCREEN_UPDATE_PARTIAL
} screen_update_t;

screen_update_t detect_screen_update_type();
int set_screen_update_type(screen_update_t screen_update);

#endif
