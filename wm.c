#include <xcb/xcb.h>
#include <unistd.h>
#include <xcb/xproto.h>
#include <xcb/randr.h>
#include <xcb/xcb_keysyms.h>
#include <stdlib.h>
#include <xcb/shape.h>
#include "wm.h"
#include "window.h"
#include "rounded_corners.h"


#define UNUSED(x) (void)(x)
#define SCRIPT "sh ~/.xwmrc"


static uint32_t values[3];

xcb_connection_t *d;
xcb_screen_t *scr;
xcb_window_t win;
Window *cur;

#include "ewmh.h"
#include "screen_data.h"

static void killclient(char **com) {
    UNUSED(com);
    Window *tmp = cur;
    while(tmp->next) {
        if(*tmp->win==win&&(!tmp->manage||tmp->dock)) {
        return;
        }
        tmp = tmp->next;
    }
    xcb_kill_client(d, win);
}

static void handleButtonPress(xcb_generic_event_t * ev) {
    xcb_button_press_event_t  * e = (xcb_button_press_event_t *) ev;
	win = e->child;
    values[0] = XCB_STACK_MODE_ABOVE;
    xcb_configure_window(d, win, XCB_CONFIG_WINDOW_STACK_MODE, values);
    values[2] = ((1 == e->detail) ? 1 : ((win != 0) ? 3 : 0 ));
    xcb_grab_pointer(d, 0, scr->root, XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION_HINT,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
        scr->root, XCB_NONE, XCB_CURRENT_TIME);
}

static void handleMotionNotify(xcb_generic_event_t * ev) {
    UNUSED(ev);
    xcb_query_pointer_cookie_t coord = xcb_query_pointer(d, scr->root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(d, coord, 0);
    Window *tmp = cur;
    while(tmp->next) {
        if(*tmp->win==win&&(!tmp->manage||tmp->dock)) {
        return;
        }
        tmp = tmp->next;
    }
    if ((values[2] == (uint32_t)(1)) && (win != 0)) {
        values[0] = poin->root_x;
        values[1] = poin->root_y;
        xcb_configure_window(d, win, XCB_CONFIG_WINDOW_X
            | XCB_CONFIG_WINDOW_Y, values);
    } else if ((values[2] == (uint32_t)(3)) && (win != 0)) {
        xcb_get_geometry_cookie_t geom_now = xcb_get_geometry(d, win);
        xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(d, geom_now, NULL);
        if (!((poin->root_x <= geom->x) || (poin->root_y <= geom->y))) {
            values[0] = poin->root_x - geom->x - 1;
            values[1] = poin->root_y - geom->y - 1;
            if ((values[0] >= (uint32_t)(480)) &&
                (values[1] >= (uint32_t)(256))) {
                xcb_configure_window(d, win, XCB_CONFIG_WINDOW_WIDTH
                    | XCB_CONFIG_WINDOW_HEIGHT, values);
            }
        }
    } else {}
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
    win = e->child;
    int key_table_size = sizeof(keys) / sizeof(*keys);
    for (int i = 0; i < key_table_size; ++i) {
        if ((keys[i].keysym == keysym) && (keys[i].mod == e->state)) {
            keys[i].func(keys[i].com);
        }
    }
}

static void handleMapRequest(xcb_generic_event_t * ev) {
    xcb_map_request_event_t * e = (xcb_map_request_event_t *) ev;
    xcb_map_window(d, e->window);
    Window *tmp = cur;
    while(tmp->next) {
        if(*tmp->win==win&&(!tmp->manage||tmp->dock)) {
        return;
        }
        tmp = tmp->next;
    }
    xcb_get_geometry_cookie_t cookie;
    xcb_get_geometry_reply_t *reply;
    cookie = xcb_get_geometry(d, e->window);
    reply = xcb_get_geometry_reply(d, cookie, NULL);
    uint32_t vals[5];
    vals[0] = get_window_x(1280);
    vals[1] = reply ? reply->y : (scr->height_in_pixels / 2) - (720 / 2);
    vals[2] = reply ? reply->width : 1280;
    vals[3] = reply ? reply->height : 720;
    vals[4] = 1;
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
    _apply_window_type(new);
    while (1) {
        if(next->next == NULL){
            next->next = new;
            break;
        }
        next = next->next;
    }
    xcb_map_window(d, e->window);
    xcb_flush(d);
}

static void handleDestroyRequest(xcb_generic_event_t *ev) {
    xcb_destroy_notify_event_t * e = (xcb_destroy_notify_event_t *) ev;

    Window *tmp = cur, *prev = cur;

    while(tmp->next) {
        if(*tmp->win==e->window&&(!tmp->manage||tmp->dock)){
        return;
        }
        tmp = tmp->next;
    }
  
    tmp = cur;

    while (tmp!=NULL && *tmp->win != e->window) { 
        prev = tmp; 
        tmp = tmp->next; 
    } 

    if(*tmp->win != e->window){
        xcb_kill_client(d, e->window);
        return;
    }

    if(*tmp->win==scr->root)
        return;

    prev->next = NULL;

    if (tmp->next)
        prev->next = tmp->next;

    free(tmp);
    xcb_kill_client(d, e->window);
}

static int eventHandler(void) {
    int ret = xcb_connection_has_error(d);
    if (ret == 0) {
        xcb_generic_event_t * ev = xcb_wait_for_event(d);
        handler_func_t * handler;
        for (handler = handler_funs; handler->func != NULL; handler++) {
            if ((ev->response_type & ~0x80) == handler->request) {
                handler->func(ev);
                window_rounded_border(win, 14);
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
    _apply_window_type(cur);
    xcb_flush(d);
    xcb_grab_button(d, 0, scr->root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, scr->root, XCB_NONE, 1, MOD1);
    xcb_grab_button(d, 0, scr->root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, scr->root, XCB_NONE, 3, MOD1);
    xcb_flush(d);
}

int main() {
    int sys = system(SCRIPT);
    if(sys)
    return 1;
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
