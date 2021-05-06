#include "window.h"
#ifndef SCREEN_DATA
#define SCREEN_DATA
extern screen_data_t *screens;
void get_screen_data();
screen_data *get_current_screen();
void ws_switch(xcb_window_t win, int right);
void win_switch(xcb_window_t win, int right);
#endif
