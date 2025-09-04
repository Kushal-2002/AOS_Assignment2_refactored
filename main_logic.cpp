#include "shell.h"
#include <iostream>
#include <sstream>
#include <termios.h>
#include <unistd.h>
using namespace std;

string input_with_history(vector<string>& command_history) {
    termios orig_termios;
    enable_raw_mode(orig_termios);

    string buffer;
    int history_index = command_history.size();
    string current_input;
    bool history_mode = false;
    static bool second_tab = false;

    cout.flush();

    
    

    while (true) {
        char ch = getchar();

        if (ch == '\n') {
            cout << endl;
            break;
        }else if (ch == 4) {  // ASCII EOT (Ctrl+D)
            if (buffer.empty()) {
                disable_raw_mode(orig_termios);
                return "__EOF__";  // special marker for EOF
            }
        } 
        else if (ch == 127 || ch == '\b') {
            if (!buffer.empty()) {
                buffer.pop_back();
                cout << "\b \b";
                cout.flush();
            }
        } else if (ch == 27) {
            char c1 = getchar();
            char c2 = getchar();
            if (c1 == '[') {
                if (c2 == 'A') {
                    if (!command_history.empty() && history_index > 0) {
                        if (!history_mode) {
                            current_input = buffer;
                            history_mode = true;
                        }
                        history_index--;
                        buffer = command_history[history_index];
                        cout << "\33[2K\r";
                        display();
                        cout << buffer;
                        cout.flush();
                    }
                } else if (c2 == 'B') {
                    if (history_mode && history_index < command_history.size() - 1) {
                        history_index++;
                        buffer = command_history[history_index];
                        cout << "\33[2K\r";
                        display();
                        cout << buffer;
                        cout.flush();
                    } else if (history_mode && history_index == command_history.size() - 1) {
                        history_index++;
                        buffer = current_input;
                        cout << "\33[2K\r";
                        display();
                        cout << buffer;
                        cout.flush();
                        history_mode = false;
                    }
                }
            }
        } else if (ch == 9) {
            string new_buffer = autocomplete(buffer, second_tab);
            if (new_buffer == buffer) {
                if (!second_tab) second_tab = true;
            } else {
                buffer = new_buffer;
                cout << "\33[2K\r";
                display();
                cout << buffer;
                cout.flush();
                second_tab = false;
            }
        } else {
            second_tab = false;
            buffer += ch;
            cout << ch;
            cout.flush();
        }
    }

    disable_raw_mode(orig_termios);
    return buffer;
}

void execute_command(vector<vector<string>> &commands, bool bg) {
    //If the command contains pipe then it goes throught this part to execute-pipeline
    if (commands.size() > 1) {
        execute_pipeline(commands, bg);
        return;
    }

    vector<string> tokens = commands[0];
    bool hasRedirection = false;
    for (auto &tok : tokens) {
        if (tok == ">" || tok == ">>" || tok == "<") {
            hasRedirection = true;
            break;
        }
    }

    if (hasRedirection) {
        system_command_handler(tokens, bg);
        return;
    }

    string cmd = tokens[0];
    if (cmd == "pwd") pwd_handler();
    else if (cmd == "cd") cd_handler(tokens);
    else if (cmd == "echo") echo_handler(tokens);
    else if (cmd == "ls") ls_handler(tokens);
    else if (cmd == "exit") exit(0);
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
    } else if (cmd == "pinfo") {
        pid_t pid = getpid();
        if (tokens.size() == 1) pinfo_handler(pid);
        else if (tokens.size() == 2) pinfo_handler(stoi(tokens[1]));
    } else if (cmd == "search") {
        if (tokens.size() != 2) cout << "Wrong arguments\n";
        else cout << (search_handler(tokens[1], ".") ? "True\n" : "False\n");
    } else {
        system_command_handler(tokens, bg);
    }
}
