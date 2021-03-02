#include <stdbool.h>

#define WORKSPACE_AMOUNT 4 
typedef struct{
    bool manage, dock, center;
}props;

typedef struct Window{
    xcb_window_t *win;
    int ws_id, scr_id;
    props rule;
    struct Window *next;
}Window;

typedef struct ID{
    int id;
    struct ID *next;
}ID;

typedef struct screen_data{
    int id, ws_id, width, height, x, y;
    struct ID *ws_list;
    struct screen_data *next;
} screen_data;

typedef struct screen_data_t{
    int len;
    struct screen_data *first;
} screen_data_t;