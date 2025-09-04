#include "shell.h"

//Used by cd to go back to the last directoy in case of -
string prev_working_directory="";
vector<string> command_history;//This keeps track of user entered commands

int main() {
    //loads history is called each time in main so that it can keep track of history across sesions

    load_history();
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    while (true) {
        display();
        string input = input_with_history(command_history);
        if (input == "__EOF__") {
            cout << "\nLogout\n";
            break;
        }
        if (input.length() == 0) continue;
        add_history(input);
        
        // 1️⃣ Split input by semicolon
        vector<string> semi_commands;
        string token;
        stringstream ss(input);
        while (getline(ss, token, ';')) {
            if (!token.empty()) semi_commands.push_back(token);
        }
        
        // for (int i=0; i<commands.size(); i++) {
            //     cout << "Command " << i << ": ";
            //     for (auto &t : commands[i]) cout << t << " ";
            //     cout << "\n";
            // }

        // 2️⃣ Handle each command separately
        for (auto &semi_cmd : semi_commands) {

            //Preprocess adds spaces around < > | >> so tokenizer splits correctly
            string processed = preprocess(semi_cmd);

            vector<string> tokens = tokenizer(processed);

            if (tokens.empty()) continue;

            //Detect backgroudn execution
            bool bg = false;
            if (!tokens.empty() && tokens.back() == "&") {
                bg = true;
                tokens.pop_back();
            }

            auto commands = split_into_commands(tokens);
            execute_command(commands, bg);
        }
    }
    return 0;
}
