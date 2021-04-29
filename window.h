#include <xcb/xcb.h>
#include <unistd.h>
#include <stdbool.h>

#ifndef WINDOW_DATA
#define WINDOW_DATA
#define WORKSPACE_AMOUNT 4 

typedef struct Window{
    xcb_window_t *win;
    int ws_id, scr_id;
    uint32_t vals[4];
    struct Window *next;
}Window;

typedef struct screen_data{
    uint32_t id, ws_id, width, height, x, y;
    struct screen_data *next;
} screen_data;

typedef struct screen_data_t{
    int len;
    struct screen_data *first;
} screen_data_t;
#endif