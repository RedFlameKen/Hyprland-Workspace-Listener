#pragma once

#include <stdint.h>

/**
 * Returns a malloc'd string
 */
char* execute_hyprctl_command(const char* restrict command);
uint8_t init_hyprctl_socket();
int8_t listen_to_socket(char* socket_path);
int8_t listen_process();
