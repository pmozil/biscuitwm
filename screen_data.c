#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <stdlib.h>
#include "screen_data.h"

screen_data_t *screens;

extern xcb_connection_t *d;
extern xcb_screen_t *scr;
extern Window *cur;
extern int workspace_amount;

#define UNUSED(x) (void)(x)

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
    screen_data *tmp = (screen_data *) calloc(1, sizeof(screen_data));
    tmp->id = i;
    tmp->y = crtc->y;
    tmp->x = crtc->x;
    tmp->width = crtc->width;
    tmp->height = crtc->height;
    tmp->next = NULL;
    tmp->ws_id=1;
    ID *ws_ids=calloc(1, sizeof(ID));
    ws_ids->id=1;
    for(int j =2; j<=WORKSPACE_AMOUNT;j++){
        ID  *ws_tmp=calloc(1, sizeof(ID));
        ws_tmp->id=j;
        ws_tmp->prev=ws_ids;
        ws_ids->next=ws_tmp;
        ws_ids=ws_ids->next;
    }
    tmp->ws_list=ws_ids;

    if(screens->first==NULL){
        screens->first=tmp;
        free(crtc);
        free(output);
        continue;
    }

    screen_data *scr_tmp=screens->first;
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

//workspace operations

void ws_switch(xcb_window_t win, bool right){
    UNUSED(win);
    screen_data *scr_tmp=get_current_screen();
    ID *tmp = scr_tmp->ws_list, *prev=tmp;
    while(tmp!=NULL&&tmp->id!=scr_tmp->ws_id){
        prev=tmp;
        tmp=tmp->next;
    }
    if(tmp==NULL||(right?tmp->next:prev)==NULL)
        return;
    Window *win_tmp=cur;
    while(win_tmp!=NULL){
        if(win_tmp->ws_id==scr_tmp->ws_id&&win_tmp->scr_id==scr_tmp->id)
            xcb_unmap_window(d, *win_tmp->win);
        win_tmp=win_tmp->next;
    }
    scr_tmp->ws_id=right?(tmp->next?tmp->id:tmp->next->id):prev->id;
    win_tmp=cur;
    while(win_tmp!=NULL){
        if(win_tmp->ws_id==scr_tmp->ws_id&&win_tmp->scr_id==scr_tmp->id)
            xcb_map_window(d, *win_tmp->win);
        win_tmp=win_tmp->next;
    }
}

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