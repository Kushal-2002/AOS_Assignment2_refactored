#include "shell.h"
#include "shell.h"
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>

void enable_raw_mode(termios &orig_termios) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void disable_raw_mode(termios &orig_termios) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}
vector<string> tokenizer(string str){
    vector<string> ans;
    char *s=strdup(str.c_str());
    char *token=strtok(s," \t");
    while(token!=NULL){
        ans.push_back(token);
        token=strtok(NULL," \t");
    }
    free(s);
    return ans;
}

string preprocess(string str){
    string res;
    for(int i=0;i<str.length();i++){
        if(str[i]=='>' || str[i]=='<' || str[i]=='|'){
            if(str[i]=='>' && i+1<str.size() && str[i+1] == '>'){
                res+=" >> ";
                i++;
            }else{
                res+=" ";
                res+=str[i];
                res+=" ";
            }
        }else{
            res+=str[i];
        }
    }
    return res;
}

void display(){
    struct passwd *password=getpwuid(getuid());
    const char *username=password->pw_name;
    char hostname[HOST_NAME_MAX];
    gethostname(hostname,sizeof(hostname));
    char cwd[PATH_MAX];
    getcwd(cwd,PATH_MAX);
    string home=getenv("HOME");
    // cout<<home<<endl;
    string cwd_as_string=cwd;
    // cout<<cwd_as_string<<endl;
    cwd_as_string="~"+cwd_as_string.substr(home.length());
    printf("%s@%s:%s>",username,hostname,cwd_as_string.c_str());
    fflush(stdout);
}

vector<vector<string>> split_into_commands(const vector<string>& tokens) {
    vector<vector<string>> commands;
    vector<string> current;
    for (auto &tok : tokens) {
        if (tok == "|") {
            if (!current.empty()) {
                commands.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(tok);
        }
    }
    if (!current.empty()) {
        commands.push_back(current);
    }
    return commands;
}

bool is_internal(string cmd) {
    return (cmd == "pwd" || cmd == "cd" || cmd == "echo" || 
            cmd == "ls" || cmd == "history" || 
            cmd == "pinfo" || cmd == "search");
}
