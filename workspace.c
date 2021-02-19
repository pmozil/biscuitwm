#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <stdlib.h>
#include "window.h"
#include "screen_data.h"

int current_workspace;
extern xcb_connection_t *d;

void switch_workspace(bool left) {
    screen_data *scr_tmp = get_current_screen();
    workspace *tmp = scr_tmp->windows, *prev=tmp;
    for(int i=0; i<tmp->current_workspace; i++){
        prev=tmp;
        tmp=tmp->next?tmp->next:tmp;
    }
    tmp->current_workspace=left?(
        (tmp->current_workspace-1>0) ? (tmp->current_workspace-1) : (tmp->current_workspace)
        ):
        tmp->current_workspace+1;
    prev = left?prev:(tmp->next?tmp->next:tmp);
    Window *temp=tmp->window_list;
    while(temp!=NULL){
        xcb_unmap_window(temp->win);
        temp = temp->next;
    }
    Window *temp=prev->window_list;
    while(temp!=NULL){
        xcb_map_window(tmp->win)
        temp = temp->next;
    }
}

void move_workspace(Window *w, bool left) {
    screen_data *scr_tmp = get_current_screen();
    workspace *tmp = scr_tmp->windows, *prev=tmp;
    for(int i=1; i<tmp->current_workspace; i++){
        prev=tmp;
        tmp=tmp->next?tmp->next:tmp;
    }
    Window *temp = tmp->window_list, *prv = tmp->window_list;

    while (temp!=NULL && w!=temp->win) { 
        prv = temp; 
        temp=temp->next?temp->next:temp;
    } 

    if(temp != w){
        return;
    }

    if(*temp->win==scr->root)
        return;

    prv->next = temp->next?temp->next:NULL;
    
    prev = left?prev:(temp->next?temp->next:temp);
    
    Window *prv = prev->window_list;
    while(prv->next){
        prv = prv->next;
    }

    prv->next = w;
   xcb_unmap_window(w->win);
}