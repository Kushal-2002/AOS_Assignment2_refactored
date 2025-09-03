#ifndef SHELL_H
#define SHELL_H

#include<iostream>
#include <fcntl.h> 
#include <unistd.h>
#include<limits.h>
#include<pwd.h>
#include<cstring>
#include<vector>
#include <dirent.h> 
#include <sys/stat.h>
#include<grp.h>
#include<ctime>
#include<string>
#include <iomanip>
#include<algorithm>
#include <sys/wait.h>
#include<sstream>
#include <termios.h>
#include <csignal>

#define MAX_HISTORY 20
#define HISTORY_FILE ".shell_command_history"

using namespace std;

// Declare the global from signals.cpp

void handle_sigtstp(int sig);
void handle_sigint(int sig);

extern pid_t fg_pid;
extern string prev_working_directory;
extern vector<string> command_history;
extern vector<string> internal_cmds;


void handle_sigtstp(int sig);
void handle_sigint(int sig);

vector<string> list_files(const string &prefix);
vector<string> get_path_commands();
string common_prefix(const vector<string> &matches);
string autocomplete(const string &current_input, bool list_all);

bool is_internal(string cmd);
void parse_redirection(vector<string> &tokens,string &input_file,string &output_file,bool &append);
void apply_redirection(const string &input_file,const string &output_file,bool append);
void enable_raw_mode(termios &orig_termios);
void disable_raw_mode(termios &orig_termios);

bool case_insensitive_cmp(const string &a, const string &b);
string permission_checker(mode_t mode);
vector<string> tokenizer(string str);
string preprocess(string str);

void pwd_handler();
void cd_handler(vector<string> tokens);
void echo_handler(vector<string> tokens);
void ls_handler(vector<string> tokens);
void ls_helper(string path,bool a_flag,bool l_flag);
void display();
vector<vector<string>> split_into_commands(const vector<string>& tokens);

void pinfo_handler(pid_t pid);
bool search_handler(const string &target,const string &dir);

void load_history();
void save_history();
void add_history(const string &command);
string input_with_history(vector<string>& command_history);

void execute_pipeline(std::vector<std::vector<std::string>> &commands, bool bg);
void system_command_handler(vector<string>& tokens, bool bg);
void execute_command(vector<vector<string>> &commands, bool bg);

#endif
