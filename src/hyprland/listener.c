#include "listener.h"
#include "workspace.h"
#include "../utils/sleep.h"
#include "../utils/string.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

static const char SOCKET_PATH_FORMAT[] = "%s/hypr/%s/.socket.sock";
static const char SOCKET2_PATH_FORMAT[] = "%s/hypr/%s/.socket2.sock";

int8_t gen_socket_path(char* restrict dest, const char* restrict socket_format){
    const char* runtime_dir = getenv("XDG_RUNTIME_DIR");
    const char* instance_sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");

    if(runtime_dir == NULL){
        printf("XDG_RUNTIME_DIR not set!\n");
        return -1;
    }

    if(instance_sig == NULL){
        printf("Hyprland is unavailable right now\n");
        return -1;
    }

    sprintf(dest, socket_format, runtime_dir, instance_sig);
    return 0;
}

int8_t create_socket_path(char* dest){
    return gen_socket_path(dest, SOCKET_PATH_FORMAT);
}

int8_t create_socket_path_2(char* dest){
    return gen_socket_path(dest, SOCKET2_PATH_FORMAT);
}

int8_t listen_to_socket(char* socket_path){
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);

    int8_t socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if(socket_fd == -1)
        return -1;

    if (connect(socket_fd, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_un)) == -1) {
        printf("Unable to connect to socket\n");
        return -1;
    }

    return socket_fd;
}

void action_workspace(uint8_t ws_number){
    char ws_buffer[WORKSPACE_BUFFER_SIZE] = {0};
    build_eww_workspace(ws_buffer, ws_number);
    printf("%s\n", ws_buffer);
}

int8_t process_workspace(char* process_buffer){
    uint8_t ws_number = process_buffer[11] - '0';
    action_workspace(ws_number);
    return 0;
}

int8_t process_monitor_switch(char* process_buffer){
    int32_t sep_index =
        str_first_instance(process_buffer, strlen(process_buffer), ',');
    int8_t ws_number = process_buffer[sep_index + 1] - '0';
    action_workspace(ws_number);
    return 0;
}

int8_t process_event(char* process_buffer){
    if(strncmp(process_buffer, "workspace>>", 11) == 0){
        return process_workspace(process_buffer);
    } else if (strncmp(process_buffer, "focusedmon>>", 12) == 0) {
        return process_monitor_switch(process_buffer);
    }
    return -1;
}

int8_t listen_process(){
    int8_t socket_fd;
    char socket_path[BUFFER_SIZE] = {0};
    create_socket_path_2(socket_path);
    if((socket_fd = listen_to_socket(socket_path)) == -1)
        return -1;

    uint8_t running = 1;
    while(running){
        char buffer[BUFFER_SIZE] = {0};

        read(socket_fd, buffer, BUFFER_SIZE);
        // if(buffer == NULL){
        //     sleep_millis(16);
        //     continue;
        // }

        process_event(buffer);
        // sleep_millis(16);
    }

    return 0;
}
