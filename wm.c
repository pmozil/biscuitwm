#include <xcb/xproto.h>
#include <xcb/randr.h>
#include <xcb/xcb_keysyms.h>
#include <stdlib.h>
#include <xcb/shape.h>
#include "wm.h"
//#include "rounded_corners.h"


#define UNUSED(x) (void)(x)
#define SCRIPT "sh ~/.xwmrc"


static uint32_t values[3];

xcb_connection_t *d;
xcb_screen_t *scr;
xcb_window_t win;
Window *cur;
screen_data_t *screens;

#include "ewmh.h"

int manage(xcb_window_t win){
    xcb_ewmh_get_atoms_reply_t win_type;
	if (xcb_ewmh_get_wm_window_type_reply(ewmh, xcb_ewmh_get_wm_window_type(ewmh, win), &win_type,NULL) == 1) {
	for(unsigned int i = 0; i<win_type.atoms_len; i++){
		xcb_atom_t a = win_type.atoms[i];
		if(a == ewmh->_NET_WM_WINDOW_TYPE_DESKTOP ||
			a == ewmh->_NET_WM_WINDOW_TYPE_NOTIFICATION||
			a == ewmh->_NET_WM_WINDOW_TYPE_DOCK)
			return 0;
		}
	}
    return 1;
}

static void killclient(xcb_window_t win, bool right) {
    Window *tmp = cur, *prev = cur;

    UNUSED(right);

    xcb_ewmh_get_atoms_reply_t win_type;
	if (xcb_ewmh_get_wm_window_type_reply(ewmh, xcb_ewmh_get_wm_window_type(ewmh, win), &win_type,NULL) == 1) {
	for(unsigned int i = 1; i<win_type.atoms_len; i++){
		xcb_atom_t a = win_type.atoms[i];
		if( a == ewmh->_NET_WM_WINDOW_TYPE_DESKTOP ||
		    a== ewmh->_NET_WM_WINDOW_TYPE_NOTIFICATION||
			a == ewmh->_NET_WM_WINDOW_TYPE_DOCK)
			return ;
		}
	}

    while (tmp!=NULL && *tmp->win != win) { 
        prev = tmp; 
        tmp = tmp->next; 
    }

    if(tmp==NULL){
        xcb_kill_client(d, win); 
        return;
    }

    if(*tmp->win==scr->root)
        return;

    prev->next = NULL;
    win = *prev->win;
    setFocus(win);
    if (tmp->next)
        prev->next = tmp->next;

    free(tmp);
    xcb_kill_client(d, win);
}

static void handleButtonPress(xcb_generic_event_t * ev) {
    xcb_button_press_event_t  * e = (xcb_button_press_event_t *) ev;
    values[0] = XCB_STACK_MODE_ABOVE;
    win = e->child;
    xcb_configure_window(d, win, XCB_CONFIG_WINDOW_STACK_MODE, values);
    values[2] = ((1 == e->detail) ? 1 : ((win != 0) ? 3 : 0 ));
    xcb_grab_pointer(d, 0, scr->root, XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION_HINT,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
        scr->root, XCB_NONE, XCB_CURRENT_TIME);
}

static void handleMotionNotify(xcb_generic_event_t * ev) {
	UNUSED(ev);
    xcb_ewmh_get_atoms_reply_t win_type;
	if (xcb_ewmh_get_wm_window_type_reply(ewmh, xcb_ewmh_get_wm_window_type(ewmh, win), &win_type,NULL) == 1) {
	for(unsigned int i = 0; i<win_type.atoms_len; i++){
		xcb_atom_t a = win_type.atoms[i];
		if( a == ewmh->_NET_WM_WINDOW_TYPE_DESKTOP ||
		    a== ewmh->_NET_WM_WINDOW_TYPE_NOTIFICATION||
			a == ewmh->_NET_WM_WINDOW_TYPE_DOCK)
			return ;
		}
	}
	xcb_query_pointer_cookie_t coord = xcb_query_pointer(d, scr->root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(d, coord, 0);
    xcb_get_geometry_cookie_t cookie;
    xcb_get_geometry_reply_t *reply;
    cookie = xcb_get_geometry(d, win);
    reply = xcb_get_geometry_reply(d, cookie, NULL);
    if ((values[2] == (uint32_t)(1)) && (win != 0)) {
        values[0] = poin->root_x - reply->width/2;
        values[1] = poin->root_y - reply->height/2;
        xcb_configure_window(d, win, XCB_CONFIG_WINDOW_X
            | XCB_CONFIG_WINDOW_Y, values);
    } else if ((values[2] == (uint32_t)(3)) && (win != 0)) {
        xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(d, win);
        xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(d, geom_now, NULL);
        if (!((poin->root_x <= geom->x || (poin->root_y <= geom->y)))) {
            values[0] = poin->root_x - geom->x - 1;
            values[1] = poin->root_y - geom->y - 1;
            if ((values[0] >= (uint32_t)(480)) &&
                (values[1] >= (uint32_t)(270))) {
                xcb_configure_window(d, win, XCB_CONFIG_WINDOW_WIDTH
                    | XCB_CONFIG_WINDOW_HEIGHT, values);
            }
        }
    } else {}
}

static void handleConfigureRequest(xcb_generic_event_t * ev){
    xcb_configure_request_event_t *e = (xcb_configure_request_event_t *) ev;
    int vals[5];
    vals[0] = e->x;
    vals[1] = e->y;
    vals[2] = e->width;
    vals[3] = e->height;
    vals[4] = e->border_width;
    xcb_configure_window(d, e->window, XCB_CONFIG_WINDOW_X |
        XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
        XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH, vals);
    xcb_flush(d);
}

static xcb_keycode_t * xcb_get_keycodes(xcb_keysym_t keysym) {
    xcb_key_symbols_t * keysyms = xcb_key_symbols_alloc(d);
    xcb_keycode_t     * keycode;
    keycode = (!(keysyms) ? NULL : xcb_key_symbols_get_keycode(keysyms, keysym));
    xcb_key_symbols_free(keysyms);
    return keycode;
}

static xcb_keysym_t xcb_get_keysym(xcb_keycode_t keycode) {
    xcb_key_symbols_t * keysyms = xcb_key_symbols_alloc(d);
    xcb_keysym_t        keysym;
    keysym = (!(keysyms) ? 0 : xcb_key_symbols_get_keysym(keysyms, keycode, 0));
    xcb_key_symbols_free(keysyms);
    return keysym;
}

static void setFocus(xcb_drawable_t window) {
    if ((window != 0) && (window != scr->root)) {
        xcb_set_input_focus(d, XCB_INPUT_FOCUS_POINTER_ROOT, window,
            XCB_CURRENT_TIME);
    }
}

static void handleEnterNotify(xcb_generic_event_t * ev) {
    xcb_enter_notify_event_t * e = ( xcb_enter_notify_event_t *) ev;
    setFocus(e->event);
}

static void handleKeyPress(xcb_generic_event_t * ev) {
    xcb_key_press_event_t * e = ( xcb_key_press_event_t *) ev;
    xcb_keysym_t keysym = xcb_get_keysym(e->detail);
    int key_table_size = sizeof(keys) / sizeof(*keys);
    for (int i = 0; i < key_table_size; ++i) {
        if ((keys[i].keysym == keysym) && (keys[i].mod == e->state)) {
            keys[i].func(win, keys[i].arg);
        }
    }
}

static void handleMapRequest(xcb_generic_event_t * ev) {
    xcb_map_request_event_t * e = (xcb_map_request_event_t *) ev;
    xcb_map_window(d, e->window);
    xcb_get_geometry_cookie_t cookie;
    xcb_get_geometry_reply_t *reply;
    cookie = xcb_get_geometry(d, e->window);
    reply = xcb_get_geometry_reply(d, cookie, NULL);
    uint32_t vals[5];
    vals[0] = reply->x?reply->x:0;
    vals[1] = reply->y?reply->y:0;
    vals[2] = reply->width?reply->width:1920;
    vals[3] = reply->height?reply->height:1080;
    vals[4] = 1;
    free(reply);
    xcb_configure_window(d, e->window, XCB_CONFIG_WINDOW_X |
        XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
        XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH, vals);
    xcb_flush(d);
    values[0] = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE;
    xcb_change_window_attributes_checked(d, e->window,
        XCB_CW_EVENT_MASK, values);
    xcb_flush(d);
}

static void handleCreateRequest(xcb_generic_event_t *ev) {
    xcb_create_notify_event_t *e = (xcb_create_notify_event_t  *) ev;
    Window *next = cur, *new = (Window *) calloc(1, sizeof(Window));
    new->win = &e->window;
    new->next = NULL;
    xcb_get_geometry_cookie_t cookie;
    xcb_get_geometry_reply_t *reply;
    cookie = xcb_get_geometry(d, e->window);
    reply = xcb_get_geometry_reply(d, cookie, NULL);
    uint32_t vals[5];
    vals[0] = reply->x?reply->x:0;
    vals[1] = reply->y?reply->y:0;
    vals[2] = reply->width?reply->width:1920;
    vals[3] = reply->height?reply->height:1080;
    vals[4] = 1;
    screen_data *scr_tmp = get_current_screen();
    new->scr_id = scr_tmp->id;
    new->ws_id = scr_tmp->ws_id;
    xcb_configure_window(d, e->window, XCB_CONFIG_WINDOW_X |
        XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
        XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH, vals);
    while (next!=NULL)
        next = next->next;
    next=new;
	ewmh_set_supporting(e->window);
    xcb_map_window(d, e->window);
    xcb_flush(d);
}

static void handleDestroyRequest(xcb_generic_event_t *ev) {
    xcb_destroy_notify_event_t * e = (xcb_destroy_notify_event_t *) ev;

    killclient(e->window, false);
    
    xcb_kill_client(d, e->window);
}

void handleClientMessage(xcb_generic_event_t *e)
{
	xcb_client_message_event_t *ev = (xcb_client_message_event_t *) e;

    if (ev->type == ewmh->_NET_CURRENT_DESKTOP) {
    	setFocus(ev->window);
		return;
	} else if (ev->type == ewmh->_NET_ACTIVE_WINDOW) {
		if ((ev->data.data32[0] == XCB_EWMH_CLIENT_SOURCE_TYPE_NORMAL) ||
		    ev->window == scr->root) {
			win = ev->window;
			setFocus(ev->window);
			return;
		}
	} else if (ev->type == ewmh->_NET_WM_DESKTOP) {
	    /*screen_data *scr_tmp = screens->first;
        for(int j=1; j<(int)ev->data.data32[0]; j++){
            scr_tmp = scr_tmp->next?scr_tmp->next:scr_tmp;
        }*/
        uint32_t vals[5];
        vals[0] = 0;
        vals[1] = 0;
        vals[2] = scr->width_in_pixels;
        vals[3] = scr->height_in_pixels;
        vals[4] = 1;
        xcb_configure_window(d, ev->window, XCB_CONFIG_WINDOW_X |
        XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
        XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH, vals);
        uint32_t state[1];
        state[0] = XCB_STACK_MODE_BELOW;
        xcb_configure_window(d, ev->window, XCB_CONFIG_WINDOW_STACK_MODE, state);
	} else if (ev->type == ewmh->_NET_CLOSE_WINDOW) {
		killclient(ev->window, false);
	}
}

static int eventHandler(void) {
    int ret = xcb_connection_has_error(d);
    if (ret == 0) {
        xcb_generic_event_t * ev = xcb_wait_for_event(d);
        handler_func_t * handler;
        for (handler = handler_funs; handler->func != NULL; handler++) {
            if ((ev->response_type & ~0x80) == handler->request) {
                handler->func(ev);
            }
        }
    }
    xcb_flush(d);
    return ret;
}

static void handleButtonRelease(xcb_generic_event_t * ev) {
    UNUSED(ev);
    xcb_ungrab_pointer(d, XCB_CURRENT_TIME);
}

static void setup(void) {
    values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
        | XCB_EVENT_MASK_PROPERTY_CHANGE;
    get_screen_data();
    ewmh_init();
    xcb_change_window_attributes_checked(d, scr->root,
        XCB_CW_EVENT_MASK, values);
    xcb_ungrab_key(d, XCB_GRAB_ANY, scr->root, XCB_MOD_MASK_ANY);
    int key_table_size = sizeof(keys) / sizeof(*keys);
    for (int i = 0; i < key_table_size; ++i) {
        xcb_keycode_t * keycode = xcb_get_keycodes(keys[i].keysym);
        if (keycode != NULL) {
            xcb_grab_key(d, 1, scr->root, keys[i].mod, *keycode,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC );
        }
    }
    cur = (Window *) calloc(1, sizeof(Window));
    cur->win = &scr->root;
    cur->next = NULL;
    xcb_flush(d);
    xcb_grab_button(d, 0, scr->root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, scr->root, XCB_NONE, 1, MOD1);
    xcb_grab_button(d, 0, scr->root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, scr->root, XCB_NONE, 3, MOD1);
    xcb_flush(d);
}

void executeRcFile(){
    sleep(1);

    int sys = system(SCRIPT);
    if(sys){
        exit(1);
    }
}
/*
void handle_state(xcb_atom_t state, unsigned int action)
{

    xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(d, win);
    xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(d, geom_now, NULL);
	if (state == ewmh->_NET_WM_STATE_FULLSCREEN) {
		if (action == XCB_EWMH_WM_STATE_ADD && (ignore_ewmh_fullscreen & STATE_TRANSITION_ENTER) == 0) {
			win->x = geom->x;
            win->y = geom->y;
            win->width =  geom->width;
            win->height =  geom->height; 
            xcb_configure_window(d, win, XCB_CONFIG_WINDOW_X |
            XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
            XCB_CONFIG_WINDOW_HEIGHT, vals);
		}
        else if (action == XCB_EWMH_WM_STATE_REMOVE && (ignore_ewmh_fullscreen & STATE_TRANSITION_EXIT) == 0) {
			if (n->client->state == STATE_FULLSCREEN) {
                uint32_t vals[4];
                
				xcb_configure_window(d, win, XCB_CONFIG_WINDOW_X |
            XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
            XCB_CONFIG_WINDOW_HEIGHT, win->vals);
			}
        }
		} else if (action == XCB_EWMH_WM_STATE_TOGGLE) {
			}
		}
		arrange(m, d);
	} else if (state == ewmh->_NET_WM_STATE_BELOW) {
		if (action == XCB_EWMH_WM_STATE_ADD) {
			set_layer(m, d, n, LAYER_BELOW);
		} else if (action == XCB_EWMH_WM_STATE_REMOVE) {
			if (n->client->layer == LAYER_BELOW) {
				set_layer(m, d, n, n->client->last_layer);
			}
		} else if (action == XCB_EWMH_WM_STATE_TOGGLE) {
			set_layer(m, d, n, n->client->layer == LAYER_BELOW ? n->client->last_layer : LAYER_BELOW);
		}
	} else if (state == ewmh->_NET_WM_STATE_ABOVE) {
		if (action == XCB_EWMH_WM_STATE_ADD) {
			set_layer(m, d, n, LAYER_ABOVE);
		} else if (action == XCB_EWMH_WM_STATE_REMOVE) {
			if (n->client->layer == LAYER_ABOVE) {
				set_layer(m, d, n, n->client->last_layer);
			}
		} else if (action == XCB_EWMH_WM_STATE_TOGGLE) {
			set_layer(m, d, n, n->client->layer == LAYER_ABOVE ? n->client->last_layer : LAYER_ABOVE);
		}
	} else if (state == ewmh->_NET_WM_STATE_HIDDEN) {
		if (action == XCB_EWMH_WM_STATE_ADD) {
			set_hidden(m, d, n, true);
		} else if (action == XCB_EWMH_WM_STATE_REMOVE) {
			set_hidden(m, d, n, false);
		} else if (action == XCB_EWMH_WM_STATE_TOGGLE) {
			set_hidden(m, d, n, !n->hidden);
		}
	} else if (state == ewmh->_NET_WM_STATE_DEMANDS_ATTENTION) {
		if (action == XCB_EWMH_WM_STATE_ADD) {
			set_urgent(m, d, n, true);
		} else if (action == XCB_EWMH_WM_STATE_REMOVE) {
			set_urgent(m, d, n, false);
		} else if (action == XCB_EWMH_WM_STATE_TOGGLE) {
			set_urgent(m, d, n, !n->client->urgent);
		}
}*/


int main() {
    executeRcFile();
	d = xcb_connect(NULL, NULL);
    int ret = xcb_connection_has_error(d);
    if (ret == 0) {
        scr = xcb_setup_roots_iterator(xcb_get_setup(d)).data;
        setup();
    }
    while (ret == 0) {
        ret = eventHandler();
    }
    xcb_flush(d);
	xcb_disconnect(d);
    return ret;
}
