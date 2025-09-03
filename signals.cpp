#include <csignal>
#include <unistd.h>
#include <iostream>
#include "shell.h"

using namespace std;

pid_t fg_pid = -1;   // define the global here

void handle_sigtstp(int sig) {
    if (fg_pid > 0) {
        kill(fg_pid, SIGTSTP);
        cout << "\nProcess stopped and moved to background\n";
        fg_pid = -1;
       
    }
}

void handle_sigint(int sig) {
    if (fg_pid > 0) {
        kill(fg_pid, SIGINT);
        cout << "\nProcess interrupted\n";
        fg_pid = -1;
    }
}
