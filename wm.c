#include <xcb/xcb.h>
#include <unistd.h>
#include <xcb/xproto.h>
#include <xcb/randr.h>
#include <xcb/xcb_keysyms.h>
#include <stdlib.h>
#include <xcb/shape.h>
#include "wm.h"
#include "rounded_corners.h"


#define UNUSED(x) (void)(x)
#define SCRIPT "sh ~/.xwmrc"


static uint32_t values[3];

xcb_connection_t *d;
xcb_screen_t *scr;
xcb_window_t win;
Window *cur;

#include "ewmh.h"


int breaker(){
    Window *tmp = cur;
    while(tmp) {
        if(*tmp->win==win&&(!tmp->rule.manage||tmp->rule.dock)) {
        return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}

static void killclient(xcb_window_t win, bool right) {
    Window *tmp = cur, *prev = cur;

    UNUSED(right);

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

    if (tmp->next)
        prev->next = tmp->next;

    free(tmp);
    xcb_kill_client(d, win);
}

static void handleButtonPress(xcb_generic_event_t * ev) {
    int breakCondition = breaker();
    if(breakCondition)
        return;
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
    int breakCondition = breaker();
    if(breakCondition)
        return;
    UNUSED(ev);
    xcb_query_pointer_cookie_t coord = xcb_query_pointer(d, scr->root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(d, coord, 0);
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
    screen_data *scr_tmp = get_current_screen();
    uint32_t vals[5];
    vals[0] = reply->x?reply->x:scr_tmp->x;
    vals[1] = reply->y?reply->y:scr_tmp->y;
    vals[2] = reply->width?reply->width:1920;
    vals[3] = reply->height?reply->height:1080;
    vals[4] = 1;
    Window *win_tmp = cur;
    while(win_tmp!=NULL && * win_tmp->win!=win)
        win_tmp=win_tmp->next;
    if(win_tmp!=NULL)
        win_tmp->scr_id=scr_tmp->id;
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
//    screen_data *scr_tmp = get_current_screen();
    new->win = &e->window;
    new->next = NULL;
    new->rule = window_props(new);
//    new->ws_id = scr_tmp->ws_id;
//    new->scr_id = scr_tmp->id;
    while (next!=NULL)
        next = next->next;
    next=new;
    xcb_map_window(d, e->window);
    xcb_flush(d);
}

static void handleDestroyRequest(xcb_generic_event_t *ev) {
    xcb_destroy_notify_event_t * e = (xcb_destroy_notify_event_t *) ev;

    killclient(e->window, false);
    
    xcb_kill_client(d, e->window);
}

static int eventHandler(void) {
    int ret = xcb_connection_has_error(d);
    if (ret == 0) {
        xcb_generic_event_t * ev = xcb_wait_for_event(d);
        handler_func_t * handler;
        handler_func * props;
        for (handler = handler_funs; handler->func != NULL; handler++) {
            if ((ev->response_type & ~0x80) == handler->request) {
                for (props = win_props; props->request != 0; props++){
                    if(handler->request==props->request){
                        xcb_flush(d);
                    }
                }
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
    cur->rule = window_props(cur);
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
