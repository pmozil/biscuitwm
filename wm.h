#include <stdbool.h>
#include "screen_data.h"

#ifndef WM
#define WM
typedef struct {
    unsigned int mod;
    xcb_keysym_t keysym;
    bool arg;
    void (*func)(xcb_window_t, bool);
} Key;

typedef struct {
    uint32_t request;
    void (*func)(xcb_generic_event_t * ev);
} handler_func_t;

#endif
#define MOD1                   XCB_MOD_MASK_4
#define MOD2                   XCB_MOD_MASK_SHIFT

static int eventHandler(void);
static void handleCreateRequest(xcb_generic_event_t *ev);
static void handleDestroyRequest(xcb_generic_event_t *ev);
static void handleMotionNotify(xcb_generic_event_t * ev);
static void handleEnterNotify(xcb_generic_event_t * ev);
static void handleButtonPress(xcb_generic_event_t * ev);
static void handleButtonRelease(xcb_generic_event_t * ev);
static void handleKeyPress(xcb_generic_event_t * ev);
static void handleMapRequest(xcb_generic_event_t * ev);
static void killclient(xcb_window_t win, bool right);
static void setFocus(xcb_window_t win);
void handleClientMessage(xcb_generic_event_t *e);
static void handleConfigureRequest(xcb_generic_event_t * ev);
int breaker();
static xcb_keycode_t * xcb_get_keycodes(xcb_keysym_t keysym);
static xcb_keysym_t    xcb_get_keysym(xcb_keycode_t keycode);
#ifndef WM_PROPS
#define WM_PROPS
static handler_func_t handler_funs[] = {
    { XCB_CREATE_NOTIFY,  handleCreateRequest },
    {XCB_CONFIGURE_REQUEST, handleConfigureRequest},
    { XCB_MOTION_NOTIFY,  handleMotionNotify },
    { XCB_ENTER_NOTIFY,   handleEnterNotify },
    { XCB_DESTROY_NOTIFY, handleDestroyRequest },
    { XCB_BUTTON_PRESS,   handleButtonPress },
    { XCB_BUTTON_RELEASE, handleButtonRelease },
    { XCB_KEY_PRESS,      handleKeyPress },
    { XCB_MAP_REQUEST,    handleMapRequest },
    { XCB_CLIENT_MESSAGE, handleClientMessage },
    { XCB_NONE,           NULL }
};

static Key keys[] = {
    { MOD1,      0x0071, false, killclient},    /* 0x0071 = XK_q */
};
#endif
