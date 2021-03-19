#include <xcb/xcb_ewmh.h>
#include "screen_data.h"

#ifndef EWMH
#define EWMH
extern xcb_ewmh_connection_t *ewmh;

void ewmh_init(void);
void ewmh_update_active_window(void);
void ewmh_update_client_list();
props window_props(Window *win);
#endif
