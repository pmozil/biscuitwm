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
	xcb_ewmh_get_atoms_reply_t win_type;\

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
	if(rule.dock||!rule.manage){
		Window *tmp=calloc(1, sizeof(Window));
		tmp->win=win->win;
		tmp->rule=rule;
		screen_data *scr_tmp=get_current_screen();
		tmp->ws_id=scr_tmp->ws_id;
		tmp->scr_id=scr_tmp->id;
		Window *win_tmp= dock_list;
		while(win_tmp!=NULL)
			win_tmp=win_tmp->next;
		win_tmp=tmp;
	}
	return rule;
}
