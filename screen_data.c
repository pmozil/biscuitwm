#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <stdlib.h>
#include "screen_data.h"

extern xcb_connection_t *d;
extern xcb_screen_t *scr;
extern Window *cur;
extern int workspace_amount;

#define UNUSED(x) (void)(x)

void append_to_screens(xcb_randr_get_crtc_info_reply_t *crtc ) {
    screen_data *tmp = (screen_data *) calloc(1, sizeof(screen_data)), *insert = screens->first;
    tmp->y = crtc->y;
    tmp->x = crtc->x;
    tmp->width = crtc->width;
    tmp->height = crtc->height;
    tmp->next = NULL;
    tmp->ws_id=1;
    uint32_t i = 1;

    while(insert!=NULL){
        insert=insert->next;
        ++i;
    }
    tmp->id=i;
    insert=tmp;
    return;
}

void get_screen_data() {
    xcb_randr_get_screen_resources_current_reply_t *reply = xcb_randr_get_screen_resources_current_reply(
    d, xcb_randr_get_screen_resources_current(d, scr->root), NULL);
    xcb_timestamp_t timestamp = reply->config_timestamp;
		xcb_randr_output_t *randr_outputs = xcb_randr_get_screen_resources_current_outputs(reply);
    int len = xcb_randr_get_screen_resources_current_outputs_length(reply);
    screens = (screen_data_t *)calloc(1, sizeof(screen_data_t));
    screens->len = len;
    for (int i = 0; i < len; i++) {
    xcb_randr_get_output_info_reply_t *output = xcb_randr_get_output_info_reply(
            d, xcb_randr_get_output_info(d, randr_outputs[i], timestamp), NULL);
    if (output == NULL)
        continue;

    if (output->crtc == XCB_NONE || output->connection == XCB_RANDR_CONNECTION_DISCONNECTED)
        continue;

    xcb_randr_get_crtc_info_reply_t *crtc = xcb_randr_get_crtc_info_reply(d,
            xcb_randr_get_crtc_info(d, output->crtc, timestamp), NULL);
    append_to_screens(crtc);
    free(crtc);
    free(output);
    }
}

screen_data *get_current_screen(){
    xcb_query_pointer_cookie_t coord = xcb_query_pointer(d, scr->root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(d, coord, 0);
    uint32_t x = poin->root_x;
    screen_data *tmp = screens->first;
    while(tmp!=NULL){
        if((tmp->x + tmp->width) >= x && (x >= tmp->x)){
            return tmp;
        }
        tmp = tmp->next;
    }
    if(tmp==NULL){
        tmp = screens->first;
    }
    return tmp;
}
