#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <stdlib.h>
#include "screen_data.h"

screen_data_t *screens;

extern xcb_connection_t *d;
extern xcb_screen_t *scr;

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
    if(!screens->iter)
        screens->iter = screens->first;
    while(screens->iter->next)
        screens->iter = screens->iter->next;
    screens->iter->next = (screen_data *) calloc(1, sizeof(screen_data));
    screens->iter->next->y = crtc->y;
    screens->iter->next->x = crtc->x;
    screens->iter->next->width = crtc->width;
    screens->iter->next->height = crtc->height;
    screens->len++;

    free(crtc);
    free(output);
    }
}

int get_window_x(int window_width) {
    xcb_query_pointer_cookie_t coord = xcb_query_pointer(d, scr->root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(d, coord, 0);
    int x = poin->root_x;
    int val;
    screens->iter = screens->first;
    for(int i=0; i<screens->len; i++){
        if(!screens->iter->next)
			break;
        if(
            ((screens->iter->x + screens->iter->width) >= x) && (x >= screens->iter->x)
        )
            break;
        
        if(screens->iter->next)
            screens->iter = screens->iter->next;
    }
    val = screens->iter->x + screens->iter->width/2 - window_width/2;
    return val;
}
