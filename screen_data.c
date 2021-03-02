#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <stdlib.h>
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
    screen_data *tmp = (screen_data *) calloc(1, sizeof(screen_data));
    tmp->id = i;
    tmp->y = crtc->y;
    tmp->x = crtc->x;
    tmp->width = crtc->width;
    tmp->height = crtc->height;
    tmp->next = NULL;
    ID *ws_tmp=tmp->ws_list;
    for(int i =0; i<WORKSPACE_AMOUNT;i++){
        ws_tmp=calloc(1, sizeof(ID));
        ws_tmp->id=i;
        ws_tmp=ws_tmp->next;
    }
    screen_data *scr_tmp = screens->first;
    while(scr_tmp!=NULL)
        scr_tmp=scr_tmp->next;
    scr_tmp=tmp;

    free(crtc);
    free(output);
    }
}

screen_data *get_current_screen() {
    xcb_query_pointer_cookie_t coord = xcb_query_pointer(d, scr->root);
    xcb_query_pointer_reply_t * poin = xcb_query_pointer_reply(d, coord, 0);
    int x = poin->root_x;
    screen_data *tmp = screens->first;
    for(int i =0; i<screens->len; i++){ 
        if((tmp->x + tmp->width) >= x && (x >= tmp->x)){
            return tmp;
        }
        tmp = tmp->next?tmp->next:tmp;
    }
    return tmp;
}

void ws_switch(screen_data *scr_switch, bool right){
    ID *tmp = scr_switch->ws_list, *prev=tmp;
    while(tmp!=NULL&&tmp->id!=scr_switch->ws_id){
        prev=tmp;
        tmp=tmp->next;
    }
    if(tmp==NULL||(right?tmp->next:prev)==NULL)
        return;
    Window *win_tmp=cur;
    while(win_tmp->next){
        if(win_tmp->ws_id==scr_switch->ws_id)
            xcb_unmap_window(d, *win_tmp->win);
        win_tmp=win_tmp->next;
    }
    scr_switch->ws_id=right?tmp->next->id:prev->id;
    win_tmp=cur;
    while(win_tmp->next){
        if(win_tmp->ws_id==scr_switch->ws_id)
            xcb_map_window(d, *win_tmp->win);
        win_tmp=win_tmp->next;
    }
}

//workspace operations

void win_switch(xcb_window_t win, bool right){
    Window *win_tmp = cur;
    while(win_tmp!=NULL&&*win_tmp->win!=win)
        win_tmp=win_tmp->next;
    
    if(win_tmp==NULL||*win_tmp->win!=win)
        return;
    screen_data *scr_tmp = screens->first;
    while(scr_tmp!=NULL&&scr_tmp->id!=win_tmp->scr_id){
        scr_tmp=scr_tmp->next;
    }
    if(scr_tmp==NULL||scr_tmp->id!=win_tmp->scr_id)
        return;
    ID *id_tmp=scr_tmp->ws_list, *prev=id_tmp;
    while(id_tmp!=NULL&&win_tmp->ws_id!=id_tmp->id){
        prev=id_tmp;
        id_tmp=id_tmp->next;
    }
    if(id_tmp==NULL||id_tmp->id!=win_tmp->ws_id)
        return;
    id_tmp=id_tmp->next?id_tmp->next:id_tmp;
    prev=right?id_tmp:prev;
    win_tmp->ws_id=prev->id;
    xcb_unmap_window(d, win);
}