#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <stdlib.h>
#include "window.h"
#include "screen_data.h"

screen_data_t *screens;

extern xcb_connection_t *d;
extern xcb_screen_t *scr;
extern Window *cur;
extern int workspace_amount;

void get_screen_data() {
    xcb_randr_get_screen_resources_current_reply_t *reply = xcb_randr_get_screen_resources_current_reply(
    d, xcb_randr_get_screen_resources_current(d, scr->root), NULL);
    xcb_randr_output_t *randr_outputs = xcb_randr_get_screen_resources_current_outputs(reply);
    xcb_timestamp_t timestamp = reply->config_timestamp;
    int len = xcb_randr_get_screen_resources_current_outputs_length(reply);
    for (int i = 0; i < len; i++) {
    xcb_randr_get_output_info_reply_t *output = xcb_randr_get_output_info_reply(
            d, xcb_randr_get_output_info(d, randr_outputs[i], timestamp), NULL);
    if (output == NULL)
        continue;

    if (output->crtc == XCB_NONE || output->connection == XCB_RANDR_CONNECTION_DISCONNECTED)
        continue;

    xcb_randr_get_crtc_info_reply_t *crtc = xcb_randr_get_crtc_info_reply(d,
            xcb_randr_get_crtc_info(d, output->crtc, timestamp), NULL);
    if(!screens) {
        screens = (screen_data_t *)calloc(1, sizeof(screen_data_t));
        screens->len = 0;
    }
    if (!screens->first) {
        screens->first =(screen_data *) calloc(1, sizeof(screen_data));
        screens->first->x = crtc->x;
        screens->first->y = crtc->y;
        screens->first->width = crtc->width;
        screens->first->height = crtc->height;
        screens->len++;
        continue;
    }
    screen_data *tmp = screens->first;
    while(tmp->next)
        tmp = tmp->next;
    tmp->next = (screen_data *) calloc(1, sizeof(screen_data));
    tmp->next->y = crtc->y;
    tmp->next->x = crtc->x;
    tmp->current_workspace=1;
    tmp->windows = calloc(1, sizeof(workspace));
    workspace *temp = tmp->windows;
    for(int i=0; i<workspace_amount; i++){
        temp->next = calloc(1, sizeof(workspace));
        temp->window_list = calloc(1, sizeof(Window));
        temp = temp->next;
    }
    tmp->next->width = crtc->width;
    tmp->next->height = crtc->height;
    tmp->next->next = NULL;
    screens->len++;

    free(crtc);
    free(output);
    }
}
//TODO: make this thingy work somehow
void insert_window(Window *w) {
    screen_data *tmp = get_current_screen();
    
    if(!tmp)
        return;

    struct workspace *temp = tmp->windows;
    for(int i =1;i<tmp->current_workspace; i++)
        temp=temp->next;

    Window *temp_win = temp->window_list;
    while(temp_win!=NULL)
        temp_win = temp_win->next;
    temp_win = w;
}

screen_data *get_current_screen() {
    xcb_query_pointer_cookie_t coord = xcb_query_pointer(d, scr->root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(d, coord, 0);
    int x = poin->root_x;
    screen_data *tmp = screens->first;
    while ((tmp->x + tmp->width) >= x && (x >= tmp->x)){
        tmp = tmp->next;
    }
    return tmp;
}
