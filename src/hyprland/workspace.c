#include "workspace.h"
#include "../utils/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>

static const char EWW_WS_OPENING[] = "(box :space-evenly true :spacing 1 ";
static const char EWW_WS_CLOSING[] = ")";

static const char WS_SYMBOL_1[] = "";
static const char WS_SYMBOL_2[] = "";
static const char WS_SYMBOL_3[] = "󰈹";
static const char WS_SYMBOL_4[] = "";
static const char WS_SYMBOL_5[] = "";
static const char WS_SYMBOL_6[] = "";
static const char WS_SYMBOL_7[] = "";
static const char WS_SYMBOL_DEFAULT[] = "󱁤";

void get_ws_symbol(char* dest, uint8_t ws_number){
    switch (ws_number) {
        case 1:
            strcpy(dest, WS_SYMBOL_1);
        case 2:
            strcpy(dest, WS_SYMBOL_2);
        case 3:
            strcpy(dest, WS_SYMBOL_3);
        case 4:
            strcpy(dest, WS_SYMBOL_4);
        case 5:
            strcpy(dest, WS_SYMBOL_5);
        case 6:
            strcpy(dest, WS_SYMBOL_6);
        case 7:
            strcpy(dest, WS_SYMBOL_7);
        default:
            strcpy(dest, WS_SYMBOL_DEFAULT);
    }
}

int8_t get_used_workspaces(uint8_t dest_buffer[10], uint8_t* ws_count){
    FILE* fd = popen("hyprctl -j workspaces | jq 'length, sort_by(.id) .[] .id'", "r");
    if(dest_buffer == NULL){
        printf("dest_buffer cannot be empty\n");
        return 1;
    }
    {
        char ws_count_buf[BUFFER_SIZE] = {0};
        fgets(ws_count_buf, BUFFER_SIZE, fd);
        *ws_count = atoi(ws_count_buf);
    }
    for(int i = 0; i < *ws_count; i++){
        char temp[BUFFER_SIZE] = {0};
        fgets(temp, BUFFER_SIZE, fd);
        dest_buffer[i] = atoi(temp);
    }
    pclose(fd);
    return 0;
}

int8_t build_eww_workspace(char* dest_buffer, uint8_t active_ws){
    if(dest_buffer == NULL){
        printf("dest_buffer cannot be empty\n");
        return 1;
    }
    char workspaces_buffer[WORKSPACE_BUFFER_SIZE] = {0};
    uint8_t ws_count = 0;
    uint8_t used_workspaces[10];
    get_used_workspaces(used_workspaces, &ws_count);
    uint8_t used_ws_index = 0;
    for(uint8_t i = 1; i <= PERSISTENT_WS_COUNT; i++){
        char ws_buffer[WORKSPACE_BUFFER_SIZE] = {0};
        uint8_t cur_used_ws = used_workspaces[used_ws_index]; 
        char used_ws_class[BUFFER_SIZE] = {0};
        if(i == cur_used_ws){
            used_ws_index++;
            strcat(used_ws_class, " ws_button_used");
        }
        char* active_ws_class = active_ws == i ? " activews" : "";
        char ws_symbol[] = {0};
        get_ws_symbol(ws_symbol, i);
        sprintf(ws_buffer, "(button :halign \"center\" :class \"ws_button%s%s\" :onclick \"hyprctl dispatch workspace %d\" (label :class \"icon_label label_w\" :text \"%s\"))", active_ws_class, used_ws_class, i, ws_symbol);
        strcat(workspaces_buffer, ws_buffer);
    }
    strcat(dest_buffer, EWW_WS_OPENING);
    strcat(dest_buffer, workspaces_buffer);
    strcat(dest_buffer, EWW_WS_CLOSING);
    return 0;
}
