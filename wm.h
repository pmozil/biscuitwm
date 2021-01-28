typedef struct {
    unsigned int mod;
    xcb_keysym_t keysym;
    void (*func)(char **com);
    char **com;
} Key;

typedef struct {
    uint32_t request;
    void (*func)(xcb_generic_event_t * ev);
} handler_func_t;

#define MOD1                   XCB_MOD_MASK_4
#define MOD2                   XCB_MOD_MASK_SHIFT

static int eventHandler(void);
static void handleMotionNotify(xcb_generic_event_t * ev);
static void handleEnterNotify(xcb_generic_event_t * ev);
static void handleDestroyNotify(xcb_generic_event_t * ev);
static void handleButtonPress(xcb_generic_event_t * ev);
static void handleButtonRelease(xcb_generic_event_t * ev);
static void handleKeyPress(xcb_generic_event_t * ev);
static void handleMapRequest(xcb_generic_event_t * ev);
static void killclient(char **com);
static xcb_keycode_t * xcb_get_keycodes(xcb_keysym_t keysym);
static xcb_keysym_t    xcb_get_keysym(xcb_keycode_t keycode);

static handler_func_t handler_funs[] = {
    { XCB_MOTION_NOTIFY,  handleMotionNotify },
    { XCB_ENTER_NOTIFY,   handleEnterNotify },
    { XCB_DESTROY_NOTIFY, handleDestroyNotify },
    { XCB_BUTTON_PRESS,   handleButtonPress },
    { XCB_BUTTON_RELEASE, handleButtonRelease },
    { XCB_KEY_PRESS,      handleKeyPress },
    { XCB_MAP_REQUEST,    handleMapRequest },
    { XCB_NONE,           NULL }
};

static Key keys[] = {
    { MOD1,      0x0071, killclient, NULL },    /* 0x0071 = XK_q */
};