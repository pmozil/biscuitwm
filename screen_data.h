#include "window.h"

extern screen_data_t *screens;
void get_screen_data();
screen_data *get_current_screen();
int get_ws_id();
void ws_switch(screen_data *scr_switch, bool right);
void win_switch(xcb_window_t win, bool right);