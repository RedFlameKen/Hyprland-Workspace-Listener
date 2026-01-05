#pragma once

#include <stdint.h>
#define BUFFER_SIZE 1024
#define READ_BUFFER_SIZE 256
#define WORKSPACE_BUFFER_SIZE 4096
#define WORKSPACE_ITEM_BUFFER_SIZE 4096

#define PERSISTENT_WS_COUNT 5

typedef struct _workspace_t {
    uint8_t is_active;
    uint8_t used;
    uint8_t number;
} workspace_t;

int8_t build_eww_workspace(char* dest_buffer);
int8_t get_workspaces(workspace_t* workspaces);
