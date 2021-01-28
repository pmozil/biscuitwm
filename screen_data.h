typedef struct screen_data{
    int width, height, x, y;
    struct screen_data *next;
} screen_data;

typedef struct screen_data_t{
    int len;
    struct screen_data *first, *last, *iter;
} screen_data_t;

extern screen_data_t *screens;
void get_screen_data();
int get_window_x(int window_width);