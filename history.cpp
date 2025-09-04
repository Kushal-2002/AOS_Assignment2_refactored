#include "shell.h"

void load_history(){
    int fd=open(HISTORY_FILE,O_RDONLY | O_CREAT ,0644);
    if(fd<0){
        cout<<"Could not open file"<<endl;
        return;
    }

    char buffer[1024];
    size_t bytes_read=read(fd,buffer,sizeof(buffer));
    string temp;
    for(size_t i=0;i<bytes_read;i++){
        if(buffer[i]=='\n'){
            if(!temp.empty()){
                command_history.push_back(temp);
                temp.clear();
            }
        }else{
            temp+=buffer[i];
        }
    }
    if(!temp.empty())command_history.push_back(temp);
    close(fd);
}
void save_history(){
    int fd=open(HISTORY_FILE,O_WRONLY | O_CREAT | O_APPEND,0644);
    if(fd<0){
        cout<<"Could not open file"<<endl;
        return;
    }
    for(auto ele:command_history){
        string cmd=ele+"\n";
        write(fd,cmd.c_str(),cmd.size());
    }
    close(fd);
}
void add_history(const string &command){
    if(!command.empty())
        command_history.push_back(command);
    if(command_history.size()>20){
        command_history.erase(command_history.begin());

    }
    save_history();
}