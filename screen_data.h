#include "window.h"
#include <xcb/xcb.h>
#include <xcb/randr.h>
#ifndef SCREEN_DATA
#define SCREEN_DATA

extern screen_data_t *screens;
void get_screen_data();
xcb_randr_get_crtc_info_reply_t *get_current_screen();
#endif