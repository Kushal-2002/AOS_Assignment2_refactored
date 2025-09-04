#include "shell.h"

void pwd_handler(){
    char cwd[PATH_MAX];
    if(getcwd(cwd,PATH_MAX)==NULL)perror("cwd failed");
    printf("%s\n",cwd);
}

void cd_handler(vector<string> tokens){
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    string current_working_directory=cwd;
    if(tokens.size()>2){
        cout<<"Invalid arguments"<<endl;
        return ;
    }
    string required_path="";
    if(tokens.size()==1 || tokens[1]=="~"){
        required_path=getenv("HOME");
    }else if(tokens[1]=="."){
        return;
    }
    else if(tokens[1]=="-"){
        required_path=prev_working_directory;
    }else if(tokens[1]==".."){
        required_path="..";
    }else{
        required_path=tokens[1];
    }   
    prev_working_directory=cwd;
    if(chdir(required_path.c_str())!=0){
        perror("cd unsuccessful");
    }
}

void echo_handler(vector<string> tokens){
    for(size_t i=1;i<tokens.size();i++){
        cout<<tokens[i];
        if(i!=tokens.size()-1)cout<<" ";
    }
    cout<<"\n";
}

string permission_checker(mode_t mode){
    string permissions="";
    permissions+=S_ISDIR(mode)?"d":"-";
    permissions+=(mode & S_IRUSR)?"r":"-";
    permissions+=(mode & S_IWUSR)?"w":"-";
    permissions+=(mode & S_IXUSR)?"x":"-";
    permissions+=(mode & S_IRGRP)?"r":"-";
    permissions+=(mode & S_IWGRP)?"w":"-";
    permissions+=(mode & S_IXGRP)?"x":"-";
    permissions+=(mode & S_IROTH)?"r":"-";
    permissions+=(mode & S_IWOTH)?"w":"-";
    permissions+=(mode & S_IXOTH)?"x":"-";
    return permissions;
}

bool case_insensitive_cmp(const string &a, const string &b) {
    string lower_a=a, lower_b=b;
    transform(a.begin(), a.end(), lower_a.begin(), ::tolower);
    transform(b.begin(), b.end(), lower_b.begin(), ::tolower);
    return lower_a < lower_b;
}

void ls_helper(string path,bool a_flag,bool l_flag){
    DIR *directory=opendir(path.c_str());
    if(!directory){
        perror("ls failed");
        return;
    }
    struct dirent *start;
    vector<string> items;
    while((start=readdir(directory))!=NULL){
        string item=start->d_name;
        items.push_back(item);
    }
    sort(items.begin(), items.end(),case_insensitive_cmp);
    if(l_flag){
        long long total=0;
        for(auto item:items){
            if(!a_flag && item[0]=='.') continue;
            struct stat st;
            string total_path=path+"/"+item;
            if(stat(total_path.c_str(),&st)==-1){
                perror("Failed to get statistics");
                return;
            }
            total+=st.st_blocks;
        }
        cout<<"total " <<total/2<<endl;
    }
    for(auto item:items){
        if(!a_flag && item[0]=='.')continue;
        if(l_flag){
            struct stat st;
            string total_path=path+"/"+item;
            if(stat(total_path.c_str(),&st)==-1){
                perror("Failed to get statistics");
            }
            struct tm *time=localtime(&st.st_mtime);    
            char last_modification_time[80];
            strftime(last_modification_time,sizeof(last_modification_time),"%b %d %H:%M",time);
            string perm=permission_checker(st.st_mode);
            cout<<perm<<" "
                << setw(3) <<st.st_nlink<<" "
                << setw(8) <<getpwuid(st.st_uid)->pw_name<<" "
                << setw(8) <<getgrgid(st.st_gid)->gr_name<<" "
                << setw(8) <<st.st_size<<" "
                << setw(12) <<last_modification_time<<" "
                <<item;
            cout<<"\n";
        }else{
            cout<<item<<" ";
        }   
    }
    if(!l_flag)cout<<"\n";
    closedir(directory);
}

void ls_handler(vector<string> tokens){
    bool a_flag=false,l_flag=false;
    for(auto &token:tokens){
        if(token[0]=='-'){
            if(token.find('a')!=string::npos)a_flag=true;
            if(token.find('l')!=string::npos)l_flag=true;
        }
    }
    string path=".";
    for(size_t i=1;i<tokens.size();i++){
        if(tokens[i][0]!='-'){
            if(tokens[i]=="~"){
                path=getenv("HOME");
            }else{
                path=tokens[i];
            }
            break;
        }
    }
    struct stat st;
    if(stat(path.c_str(), &st) == -1){
        perror("ls failed");
        return;
    }
    if(S_ISDIR(st.st_mode)){
        ls_helper(path,a_flag,l_flag);
    }else{
        if(l_flag){
            struct tm *time=localtime(&st.st_mtime);    
            char last_modification_time[80];
            strftime(last_modification_time,sizeof(last_modification_time),"%b %d %H:%M",time);
            string perm=permission_checker(st.st_mode);
            cout<<perm<<" "
                << setw(3) <<st.st_nlink<<" "
                << setw(8) <<getpwuid(st.st_uid)->pw_name<<" "
                << setw(8) <<getgrgid(st.st_gid)->gr_name<<" "
                << setw(8) <<st.st_size<<" "
                << setw(12) <<last_modification_time<<" "
                <<path;
            cout<<"\n";
        }else{
            cout<<path<<" ";
        }
    }
}

void pinfo_handler(pid_t pid){
    string path="/proc/"+to_string(pid)+"/stat";
    
    int fd=open(path.c_str(),O_RDONLY);
    if(fd<0){
        perror("Error opening file");
        return;
    }

    char buffer[1024];

    ssize_t n=read(fd,buffer,1023);

    if(n < 0){
        perror("Error reading /proc/stat");
        close(fd);
        return;
    }
    buffer[n] = '\0';
    close(fd);

    stringstream ss(buffer);
    
    int pid_read, ppid, pgrp;
    string comm;
    char status;
    
    ss>>pid_read>>comm>>status>>ppid>>pgrp;
    
    // check foreground
    pid_t fg_pgrp = tcgetpgrp(STDIN_FILENO);
    
    cout << "Process Status -- " << status;
    if (pgrp == fg_pgrp) {
        cout << "+";
    }
    cout << endl;

    //Getting virtual memory size used

    path="/proc/"+to_string(pid)+"/statm";

    fd = open(path.c_str(), O_RDONLY);

    if(fd < 0){
        perror("Error opening /proc/statm");
        return;
    }
    n = read(fd, buffer, sizeof(buffer)-1);
    if(n < 0){
        perror("Error reading /proc/statm");
        close(fd);
        return;
    }
    buffer[n] = '\0';
    close(fd);
    
    stringstream m_ss(buffer);
    
    int vm_pages;
    
    long page_size = sysconf(_SC_PAGESIZE); 

    m_ss>>vm_pages;
    
    //VM size in bytes
    
    cout << "memory -- " << vm_pages*page_size << " {Virtual Memory}" << endl;

    //Getting executable path
    path="/proc/"+to_string(pid)+"/exe";
    
    char exe_path[4096];
    
    ssize_t len=readlink(path.c_str(),exe_path,sizeof(exe_path)-1);
    
    if(len!=-1){
        exe_path[len]='\0';
    }
    cout << "Executable Path -- " << exe_path << endl;
    // cout<<content<<endl;
}

bool search_handler(const string &target,const string &dir){
    DIR *dp=opendir(dir.c_str());
    if(!dp)return false;
    struct dirent *ele;
    while((ele=readdir(dp))!=NULL){
        string name=ele->d_name;
        if(name=="." || name=="..")continue;
        string fullpath=dir+ "/"+name;
        if(name==target){
            closedir(dp);
            return true;
        }
        struct stat st;
        if(stat(fullpath.c_str(),&st)==0 && S_ISDIR(st.st_mode)){
            if(search_handler(target,fullpath)){
                closedir(dp);
                return true;
            }
        }
    }
    closedir(dp);
    return false;
}
