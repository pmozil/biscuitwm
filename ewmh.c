#include <xcb/xcb.h>
#include <stdlib.h>
#include "window.h"
#include "ewmh.h"
#include "screen_data.h"

xcb_ewmh_connection_t *ewmh;
extern xcb_connection_t *d;
extern xcb_screen_t *scr;
extern xcb_window_t win;
extern Window *cur;

void ewmh_init(void) {
	ewmh = calloc(1, sizeof(xcb_ewmh_connection_t));
	if (xcb_ewmh_init_atoms_replies(ewmh, xcb_ewmh_init_atoms(d, ewmh), NULL) == 0) {
		exit(1);
	}
}

void ewmh_update_active_window(void) {
	xcb_ewmh_set_active_window(ewmh, screens->len, win);
}

void ewmh_update_client_list() {
    int iter = 0, len = 1;

    while(cur->next) {
        len++;
    }

	if (len == 0) {
		xcb_ewmh_set_client_list(ewmh, screens->len, 0, NULL);
		xcb_ewmh_set_client_list_stacking(ewmh, screens->len, 0, NULL);
		return;
	}

    xcb_window_t wins[len];

	while(cur->next) {
        wins[iter] = *cur->win;
		iter++;
    }

    xcb_ewmh_set_client_list(ewmh, screens->len, len, wins);
}

void _apply_window_type(Window *win) {
	xcb_ewmh_get_atoms_reply_t win_type;
	win->manage = true;
	if (xcb_ewmh_get_wm_window_type_reply(ewmh, xcb_ewmh_get_wm_window_type(ewmh, *win->win), &win_type, NULL) == 1) {
		for (unsigned int i = 0; i < win_type.atoms_len; i++) {
			xcb_atom_t a = win_type.atoms[i];
			if (a == ewmh->_NET_WM_WINDOW_TYPE_DOCK ||
			    a == ewmh->_NET_WM_WINDOW_TYPE_DESKTOP ||
			    a == ewmh->_NET_WM_WINDOW_TYPE_NOTIFICATION) {
				win->manage = false;
				if(a == ewmh->_NET_WM_WINDOW_TYPE_DOCK)
					win->dock = true;
			}
		}
		xcb_ewmh_get_atoms_reply_wipe(&win_type);
	}
}