#include "workspace.h"
#include "ipc.h"
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

static const char EWW_WS_OPENING[] = "(box :space-evenly true :spacing 1 ";
static const char EWW_WS_CLOSING[] = ")";

static const char* WS_SYMBOLS[] = {
    "󰈹",
    "",
    "",
    "",
    "",
    "",
    "",
    "󱁤"
};


const char* get_ws_symbol(uint8_t ws_number){
    uint8_t array_size = sizeof(WS_SYMBOLS)/sizeof(WS_SYMBOLS[0]);
    if(ws_number >= array_size)
        return WS_SYMBOLS[array_size-1];
    else
        return WS_SYMBOLS[ws_number-1];
}

int sort_by_id(const void* a, const void* b){
    json_object *const *j1 = a, *const *j2 = b;
    json_object *id1_obj, *id2_obj;

    if(!*j1 && !*j2)
        return 0;

    if(!*j1)
        return -1;

    if(!*j2)
        return 1;

    if(!json_object_object_get_ex(*j1, "id", &id1_obj)){
        return 0;
    }
    if(!json_object_object_get_ex(*j2, "id", &id2_obj)){
        return 0;
    }
    uint8_t id1 = json_object_get_int(id1_obj);
    uint8_t id2 = json_object_get_int(id2_obj);

    return id1 - id2;
}

int8_t get_active_ws(){
    char* output = execute_hyprctl_command("[-j]/monitors");
    json_object* json = json_tokener_parse(output);
    uint8_t id;

    if(json_object_get_type(json) != json_type_array){
        id = 0;
        goto exit_node;
    }

    size_t len = json_object_array_length(json);
    json_object* focused_monitor = NULL;
    for (int i = 0; i < len; i++) {
        json_object* monitor = json_object_array_get_idx(json, i);
        json_object* focused_obj;
        if(!json_object_object_get_ex(monitor, "focused", &focused_obj)){
            id = -1;
            goto exit_node;
        }
        uint8_t is_focused = json_object_get_boolean(focused_obj);
        if(!is_focused){
            continue;
        }
        focused_monitor = monitor;
    }

    json_object *active_ws, *id_obj;
    if(json_object_object_get_ex(focused_monitor, "activeWorkspace", &active_ws) == 0){
        id = -1;
        goto exit_node;
    }

    if(json_object_object_get_ex(active_ws, "id", &id_obj) == 0){
        id = -1;
        goto exit_node;
    }

    id = json_object_get_int(id_obj);

exit_node:
    json_object_put(json);
    free(output);
    return id;
}

workspace_t create_empty_ws(uint8_t number){
    workspace_t ws = {
        .number = number,
        .is_active = 0,
        .used = 0
    };
    return ws;
}

void init_workspaces(workspace_t* workspaces){
    for (uint8_t i = 1; i <= 10; i++) {
        if(i <= PERSISTENT_WS_COUNT)
            workspaces[i-1] = create_empty_ws(i);
        else
            workspaces[i-1] = create_empty_ws(0);
    }
}

int8_t get_workspaces(workspace_t* workspaces){
    init_workspaces(workspaces);
    char* output = execute_hyprctl_command("[-j]/workspaces");
    json_object* json = json_tokener_parse(output);
    uint8_t ret = 0;

    if(json_object_get_type(json) != json_type_array){
        ret = -1;
        goto exit_node;
    }

    json_object_array_sort(json, sort_by_id);
    size_t len = json_object_array_length(json);
    size_t cur_ws = 0;
    size_t persistent_ws_count = 0;
    for (int i = 0; i < len; i++) {
        json_object* ws_item = json_object_array_get_idx(json, i);
        json_object* id_obj;
        json_object_object_get_ex(ws_item, "id", &id_obj);
        uint8_t id = json_object_get_int(id_obj);
        workspace_t ws = {.is_active=0, .number=id, .used=1};
        workspaces[id-1] = ws;
    }

    uint8_t active_ws = get_active_ws();
    workspaces[active_ws-1].is_active = 1;
    workspaces[active_ws-1].number = active_ws;

exit_node:
    json_object_put(json);
    free(output);
    return ret;
}

int8_t build_eww_workspace(char* dest_buffer){
    char output_buffer[WORKSPACE_BUFFER_SIZE] = {0};
    workspace_t workspaces[10];
    get_workspaces(workspaces);

    for (uint8_t i = 0; i < 10; i++) {
        workspace_t ws = workspaces[i];
        if(ws.number == 0){
            continue;
        }
        char* active_ws_class = ws.is_active ? " activews" : "";
        char* used_ws_class = ws.used ? " usedws" : "";
        const char* ws_symbol = get_ws_symbol(ws.number);
        char buffer[WORKSPACE_ITEM_BUFFER_SIZE] = {0};
        sprintf(buffer,
                "(eventbox :space-evenly true :halign \"center\" :class "
                "\"ws_button%s%s\" :cursor \"pointer\" :onclick "
                "\"hyprctl dispatch workspace %d\" (label :class \"icon_label "
                "label_w\" :text \"%s\" :halign \"center\" :valign \"center\" "
                ":justify \"center\"))",
                active_ws_class, used_ws_class, ws.number, ws_symbol);
        strcat(output_buffer, buffer);
    }

    strcat(dest_buffer, EWW_WS_OPENING);
    strcat(dest_buffer, output_buffer);
    strcat(dest_buffer, EWW_WS_CLOSING);
    return 0;
}
