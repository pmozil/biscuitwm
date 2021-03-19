#include <xcb/xcb.h>
#include <stdlib.h>
#include "ewmh.h"

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

props window_props(Window *win) {
	xcb_ewmh_get_atoms_reply_t win_type;

	props rule;
	rule.manage=true;
	rule.dock=false;
	rule.center=false;

	if (xcb_ewmh_get_wm_window_type_reply(ewmh, xcb_ewmh_get_wm_window_type(ewmh, *win->win), &win_type, NULL) == 1) {
		for (unsigned int i = 0; i < win_type.atoms_len; i++) {
			xcb_atom_t a = win_type.atoms[i];
			if (
			    a == ewmh->_NET_WM_WINDOW_TYPE_DESKTOP ||
			    a == ewmh->_NET_WM_WINDOW_TYPE_NOTIFICATION) {
				rule.manage=false;
			}
			if (a == ewmh->_NET_WM_WINDOW_TYPE_DOCK) {
				rule.dock=true;
			}
		xcb_ewmh_get_atoms_reply_wipe(&win_type);
		}
	}
	return rule;
}
/*void ewmh_wm_state_update(Window *w) {
     size_t count = 0;
     uint32_t values[12];
 #define HANDLE_WM_STATE(s)  \
     if (WM_FLAG_##s & n->wm_flags) { \
         values[count++] = ewmh->_NET_WM_STATE_##s; \
     }
     HANDLE_WM_STATE(MODAL)
     HANDLE_WM_STATE(STICKY)
     HANDLE_WM_STATE(MAXIMIZED_VERT)
     HANDLE_WM_STATE(MAXIMIZED_HORZ)
     HANDLE_WM_STATE(SHADED)
     HANDLE_WM_STATE(SKIP_TASKBAR)
     HANDLE_WM_STATE(SKIP_PAGER)
     HANDLE_WM_STATE(HIDDEN)
     HANDLE_WM_STATE(FULLSCREEN)
     HANDLE_WM_STATE(ABOVE)
     HANDLE_WM_STATE(BELOW)
     HANDLE_WM_STATE(DEMANDS_ATTENTION)
 #undef HANDLE_WM_STATE
     xcb_ewmh_set_wm_state(ewmh, w->win, count, values);
}

void ewmh_set_supporting(xcb_window_t win) {
	pid_t wm_pid = getpid();
	xcb_ewmh_set_supporting_wm_check(ewmh, scr->root, win);
     xcb_ewmh_set_supporting_wm_check(ewmh, win, win);
     xcb_ewmh_set_wm_name(ewmh, win, strlen(WM_NAME), WM_NAME);
     xcb_ewmh_set_wm_pid(ewmh, win, wm_pid);
}*/
