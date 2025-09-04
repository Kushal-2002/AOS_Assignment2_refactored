#include "shell.h"

void execute_pipeline(vector<vector<string>> &commands, bool bg) {
    size_t n = commands.size();
    int pipefds[2 * (n - 1)];
    for (size_t i = 0; i < n - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return;
        }
    }
    vector<pid_t> pids;
    pids.reserve(n);

    
    for (size_t i = 0; i < n; i++) {
        pid_t pid = fork();
        // Inside the fork loop, in the parent branch:
        if (pid > 0) {
            pids.push_back(pid);
        }   
        if (pid == 0) {
            //Last command is handled
            if (i > 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            //First command is handled
            if (i < n - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }
            for (size_t j = 0; j < 2 * (n - 1); j++) {
                close(pipefds[j]);
            }
            //This part is for handling io redirecton if present inside pipeline
            for (size_t j = 0; j < commands[i].size(); j++) {
                if (commands[i][j] == "<" && j + 1 < commands[i].size()) {
                    int fd_in = open(commands[i][j + 1].c_str(), O_RDONLY);
                    if (fd_in < 0) { 
                        perror("input redirection could not be done"); exit(1);
                    }
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                    commands[i].erase(commands[i].begin() + j, commands[i].begin() + j + 2);
                    j--;
                } 
                else if ((commands[i][j] == ">" || commands[i][j] == ">>") && j + 1 < commands[i].size()) {
                    bool append = (commands[i][j] == ">>");
                    int fd_out = open(commands[i][j + 1].c_str(),
                                      O_CREAT | O_WRONLY | (append ? O_APPEND : O_TRUNC),
                                      0644);
                    if (fd_out < 0) { perror("output redirection"); exit(1); }
                    dup2(fd_out, STDOUT_FILENO);
                    close(fd_out);
                    commands[i].erase(commands[i].begin() + j, commands[i].begin() + j + 2);
                    j--;
                }
            }
            if (!commands[i].empty()) {
                string cmd = commands[i][0];
                //This is the place from where internal commands will be handled
                if (is_internal(cmd)) {
                    if (cmd == "cd") cd_handler(commands[i]);
                    else if (cmd == "pwd") pwd_handler();
                    else if (cmd == "echo") echo_handler(commands[i]);
                    else if (cmd == "ls") ls_handler(commands[i]);
                    else if (cmd == "history") {
                        int N = command_history.size();
                        int num = 10;
                        if (commands[i].size() > 1) {
                            num = stoi(commands[i][1]);
                            if (num < 0) num = 0;
                            if (num > N) num = N;
                        }
                        int start = max(0, N - num);
                        for (int k = start; k < N; k++) {
                            cout << k + 1 << " " << command_history[k] << endl;
                        }
                    }
                    else if (cmd == "pinfo") {
                        pid_t pid = getpid();
                        if (commands[i].size() == 1) pinfo_handler(pid);
                        else if (commands[i].size() == 2) pinfo_handler(stoi(commands[i][1]));
                    }
                    else if (cmd == "search") {
                        if (commands[i].size() != 2) {
                            cout << "Wrong arguments" << endl;
                        } else if (search_handler(commands[i][1], ".")) {
                            cout << "True" << endl;
                        } else {
                            cout << "False" << endl;
                        }
                    }
                    exit(0);
                } else {
                    //If the command is not built by me then execvp will handle it
                    vector<char*> argv;
                    for (auto &s : commands[i]) argv.push_back(const_cast<char*>(s.c_str()));
                    argv.push_back(NULL);
                    if (execvp(argv[0], argv.data()) == -1) {
                        perror(argv[0]);   
                        exit(1);
                    }
                }
            }
            exit(0);
        }
    }
    for (size_t j = 0; j < 2 * (n - 1); j++) {
        close(pipefds[j]);
    }
    if (!bg) {
        //We are waiting for all each specific child PID we just forked
        for (pid_t cpid : pids) {
            int status;
            // Wait for this specific child; retry if interrupted by a signal
            while (waitpid(cpid, &status, 0) == -1) { /* retry */ }
        }
    }
}
