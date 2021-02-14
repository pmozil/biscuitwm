#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <stdlib.h>
#include "window.h"
#include "screen_data.h"

screen_data_t *screens;

extern xcb_connection_t *d;
extern xcb_screen_t *scr;
extern Window *cur;

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
    tmp->next->width = crtc->width;
    tmp->next->height = crtc->height;
    tmp->next->next = NULL;
    screens->len++;

    free(crtc);
    free(output);
    }
}

void insert_window(Window *w) {
    xcb_query_pointer_cookie_t coord = xcb_query_pointer(d, scr->root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(d, coord, 0);
    int x = poin->root_x;
    screen_data *tmp = screens->first;
    while(tmp){
        if(
            ((tmp->x + tmp->width) >= x) && (x >= tmp->x)
        ){
            Window *temp = tmp->windows->window_list;
            while(temp->next)
                temp = temp->next;
            temp->next = w;
            break;
        }
        tmp = tmp->next;
    }
}
