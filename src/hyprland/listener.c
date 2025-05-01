#include "listener.h"
#include "../utils/sleep.h"
#include "../utils/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFFER_SIZE 1024

int8_t listen_to_socket(uint8_t* socket_fd){
    const char* instance_sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
    if(instance_sig == NULL){
        printf("Hyprland is unavailable right now\n");
        return 1;
    }

    const char* runtime_dir = getenv("XDG_RUNTIME_DIR");
    struct sockaddr_un addr;
    *socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    addr.sun_family = AF_UNIX;

    char socket_path[BUFFER_SIZE] = {0};
    sprintf(socket_path, "%s/hypr/%s/.socket2.sock", runtime_dir, instance_sig);
    strcpy(addr.sun_path, socket_path);

    if (connect(*socket_fd, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_un)) == -1) {
        printf("Unable to connect to socket\n");
        return 1;
    }

    return 0;
}

void action_workspace(uint8_t ws_number){
    printf("workspace: %d\n", ws_number);
}

int8_t process_workspace(char* process_buffer){
    uint8_t ws_number = process_buffer[11] - '0';
    action_workspace(ws_number);
    return 0;
}

int8_t process_monitor_switch(char* process_buffer){
    printf("switched monitor\n");
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
    uint8_t socket_fd;
    if(listen_to_socket(&socket_fd) == -1){
        return -1;
    }

    FILE* fd = fdopen(socket_fd, "r");
    uint8_t running = 1;
    while(running){
        char buffer[BUFFER_SIZE] = {0};

        char* buf_ptr = fgets(buffer, BUFFER_SIZE, fd);
        if(buf_ptr == NULL){
            sleep_millis(16);
            continue;
        }

        process_event(buffer);
        sleep_millis(16);
    }

    return 0;
}
