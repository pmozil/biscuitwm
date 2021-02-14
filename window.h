#include <stdbool.h>

typedef struct{
    bool manage, dock, center;
}props;

typedef struct Window{
    xcb_window_t *win;
    props rule;
    struct Window *next;
}Window;

typedef struct {
    Window *window_list;
    xcb_drawable_t *root;
    struct workspace *next;
} workspace;

typedef struct screen_data{
    int width, height, x, y;
    workspace *windows;
    struct screen_data *next;
} screen_data;

typedef struct screen_data_t{
    int len;
    struct screen_data *first;
} screen_data_t;