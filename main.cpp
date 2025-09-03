#include "shell.h"


string prev_working_directory="";
vector<string> command_history;

int main(){
    
    signal(SIGTSTP, handle_sigtstp);  // CTRL+Z
    signal(SIGINT, handle_sigint); 
    load_history();
    while(true){

        display();
        string input;
        // getline(cin, input);

        input=input_with_history(command_history);
        if(input.length()==0)continue;
        add_history(input);

        //For detecting the < << > etc
        input=preprocess(input);
        vector<string> tokens=tokenizer(input);
    
        // for (int i=0; i<commands.size(); i++) {
            //     cout << "Command " << i << ": ";
            //     for (auto &t : commands[i]) cout << t << " ";
            //     cout << "\n";
            // }
            
            bool bg = false;
            if (!tokens.empty() && tokens.back() == "&") {
                bg = true;
                tokens.pop_back();
            }
            
        auto commands=split_into_commands(tokens);
            
        execute_command(commands,bg);
        
    }
    return 0;
}
