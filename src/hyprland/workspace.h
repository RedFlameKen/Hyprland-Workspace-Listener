#pragma once

#include <stdint.h>
#define BUFFER_SIZE 1024
#define WORKSPACE_BUFFER_SIZE 4098

#define PERSISTENT_WS_COUNT 5

int8_t get_used_workspaces(uint8_t dest_buffer[10], uint8_t* ws_count);
int8_t build_eww_workspace(char* dest_buffer, uint8_t active_ws);
