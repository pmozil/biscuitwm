#include <xcb/xcb_ewmh.h>
#include "screen_data.h"

extern xcb_ewmh_connection_t *ewmh;
extern Window *dock_list;

void ewmh_init(void);
void ewmh_update_active_window(void);
void ewmh_update_client_list();
props window_props(Window *win);