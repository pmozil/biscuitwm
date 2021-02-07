#include <stdbool.h>

typedef struct Window{
    xcb_window_t *win;
    bool manage;
    struct Window *next;
}Window;