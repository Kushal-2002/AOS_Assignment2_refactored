#include "shell.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstring>
#include <iostream>

void system_command_handler(vector<string>& tokens, bool bg) {
    string input_file = "", output_file = "";
    bool append = false;

    // Extract and strip redirections
    parse_redirection(tokens, input_file, output_file, append);

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return; }

    if (pid == 0) {  // --- CHILD ---
        apply_redirection(input_file, output_file, append);

        if (!tokens.empty()) {
            string cmd = tokens[0];

            if (is_internal(cmd)) {
                if (cmd == "cd") cd_handler(tokens);
                else if (cmd == "pwd") pwd_handler();
                else if (cmd == "echo") echo_handler(tokens);
                else if (cmd == "ls") ls_handler(tokens);
                else if (cmd == "history") {
                    int N = command_history.size();
                    int num = 10;
                    if (tokens.size() > 1) {
                        num = stoi(tokens[1]);
                        num = min(num, N);
                        if (num < 0) num = 0;
                    }
                    int start = max(0, N - num);
                    for (int i = start; i < N; i++) {
                        cout << i + 1 << " " << command_history[i] << endl;
                    }
                }
                else if (cmd == "pinfo") {
                    pid_t pid = getpid();
                    if (tokens.size() == 1) pinfo_handler(pid);
                    else if (tokens.size() == 2) pinfo_handler(stoi(tokens[1]));
                }
                else if (cmd == "search") {
                    if (tokens.size() != 2) cout << "Wrong arguments\n";
                    else cout << (search_handler(tokens[1], ".") ? "True\n" : "False\n");
                }
                exit(0); // Done internal
            } else {
                // External command
                vector<char*> argv;
                for (auto &s : tokens) argv.push_back(strdup(s.c_str()));
                argv.push_back(NULL);
                if (execvp(argv[0], argv.data()) == -1) {
                    perror(argv[0]);   
                    exit(1);
                }

            }
        }
        exit(0);
    }
    else {  // --- PARENT ---
        if (!bg) {
            fg_pid = pid;  // tell handlers which process is foreground
            waitpid(pid, NULL, WUNTRACED);  
            fg_pid = -1;   //  reset once child exits
        } else {
            cout << "PID of background process: " << pid << endl;
        }
    }

}


void parse_redirection(vector<string> &tokens,
                       string &input_file,
                       string &output_file,
                       bool &append) {
    vector<string> cleaned;
    for (int i = 0; i < tokens.size(); i++) {
        if (tokens[i] == "<" && i + 1 < tokens.size()) {
            input_file = tokens[++i];
        }
        else if (tokens[i] == ">" && i + 1 < tokens.size()) {
            output_file = tokens[++i];
            append = false;
        }
        else if (tokens[i] == ">>" && i + 1 < tokens.size()) {
            output_file = tokens[++i];
            append = true;
        }
        else {
            cleaned.push_back(tokens[i]);
        }
    }
    tokens = cleaned;
}

void apply_redirection(const string &input_file,
                       const string &output_file,
                       bool append) {
    if (!input_file.empty()) {
        int fd_in = open(input_file.c_str(), O_RDONLY);
        if (fd_in < 0) { perror("open input"); exit(1); }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    if (!output_file.empty()) {
        int fd_out = open(output_file.c_str(),
                          O_CREAT | O_WRONLY | (append ? O_APPEND : O_TRUNC),
                          0644);
        if (fd_out < 0) { perror("open output"); exit(1); }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
}
