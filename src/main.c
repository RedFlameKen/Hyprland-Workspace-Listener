#include "hyprland/ipc.h"
#include <signal.h>
#include <stdio.h>

void sigpipe_handle(int sig){
    printf("received SIGPIPE\n");
}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, sigpipe_handle);
    listen_process();
    return 0;
}
