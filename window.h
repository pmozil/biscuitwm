#include <stdbool.h>

typedef struct Window{
    xcb_window_t *win;
    bool manage, dock;
    struct Window *next;
}Window;