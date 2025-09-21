#include "ipc.h"
#include "workspace.h"
#include <asm-generic/socket.h>
#include <linux/prctl.h>
#include <signal.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define READ_STEP 128
#define WORKSPACE_REPLY_SIZE 2048

static const char SOCKET_PATH_FORMAT[] = "%s/hypr/%s/.socket.sock";
static const char SOCKET2_PATH_FORMAT[] = "%s/hypr/%s/.socket2.sock";

static uint8_t running = 1;

void on_parent_death(int signal){
    running = 0;
}

int8_t gen_socket_path(char* restrict dest, const char* restrict socket_format, char* runtime_dir, char* instance_sig){
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

int8_t create_socket_path(char* restrict dest, char* restrict runtime_dir, char* restrict instance_sig){
    return gen_socket_path(dest, SOCKET_PATH_FORMAT, runtime_dir, instance_sig);
}

int8_t create_socket_path_2(char* restrict dest, char* restrict runtime_dir, char* restrict instance_sig){
    return gen_socket_path(dest, SOCKET2_PATH_FORMAT, runtime_dir, instance_sig);
}

uint8_t connect_to_socket(char* socket_path){
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);

    int8_t socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if(socket_fd == -1)
        return -1;

    if (connect(socket_fd, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_un)) == -1) {
        printf("Unable to connect to socket: %s\n", socket_path);
        return -1;
    }

    return socket_fd;
}

uint8_t init_hyprctl_socket(){
    char* runtime_dir = getenv("XDG_RUNTIME_DIR");
    char* instance_sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");

    char* socket_path = malloc(sizeof(char)*
            (strlen(runtime_dir) + strlen(instance_sig) +
             strlen(SOCKET2_PATH_FORMAT) - 4));
    create_socket_path(socket_path, runtime_dir, instance_sig);

    uint8_t socket_fd = connect_to_socket(socket_path);

    free(socket_path);
    return socket_fd;
}

/**
 * Returns a malloc'd string
 */
char* execute_hyprctl_command(const char* restrict command){
    uint8_t socket_fd = init_hyprctl_socket(); 
    write(socket_fd, command, strlen(command));

    char* out_buffer = malloc(sizeof(char)*WORKSPACE_REPLY_SIZE);
    read(socket_fd, out_buffer, WORKSPACE_REPLY_SIZE);
    close(socket_fd);

    return out_buffer;
}

void action_workspace(){
    char ws_buffer[WORKSPACE_BUFFER_SIZE] = {0};
    build_eww_workspace(ws_buffer);
    printf("%s\n", ws_buffer);
    fflush(stdout);
}

int8_t listen_process(){
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    signal(SIGHUP, on_parent_death);
    char* runtime_dir = getenv("XDG_RUNTIME_DIR");
    char* instance_sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");

    char* socket_path = malloc(sizeof(char)*
            (strlen(runtime_dir) + strlen(instance_sig) +
             strlen(SOCKET2_PATH_FORMAT) - 4));
    create_socket_path_2(socket_path, runtime_dir, instance_sig);

    uint8_t socket2_fd = connect_to_socket(socket_path);

    free(socket_path);

    action_workspace();
    while(running){
        char buffer[READ_BUFFER_SIZE] = {0};

        read(socket2_fd, buffer, READ_BUFFER_SIZE);

        action_workspace();
    }

    close(socket2_fd);

    return 0;
}
