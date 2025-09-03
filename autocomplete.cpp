#include "shell.h"

vector<string> internal_cmds = {"pwd","cd","echo","ls","history","pinfo","search"};

vector<string> list_files(const string &prefix) {
    vector<string> matches;
    DIR *dp = opendir(".");
    if (!dp) return matches;

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        string name = entry->d_name;
        if (name.find(prefix) == 0) matches.push_back(name);
    }
    closedir(dp);
    sort(matches.begin(), matches.end());
    return matches;
}

vector<string> get_path_commands() {
    vector<string> cmds;
    char *path_env = getenv("PATH");
    if (!path_env) return cmds;

    string path_str(path_env);
    stringstream ss(path_str);
    string dir;

    while (getline(ss, dir, ':')) {
        DIR *dp = opendir(dir.c_str());
        if (!dp) continue;
        struct dirent *entry;
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
                cmds.push_back(entry->d_name);
            }
        }
        closedir(dp);
    }
    sort(cmds.begin(), cmds.end());
    cmds.erase(unique(cmds.begin(), cmds.end()), cmds.end());
    return cmds;
}

string common_prefix(const vector<string> &matches) {
    if (matches.empty()) return "";
    string prefix = matches[0];
    for (size_t i = 1; i < matches.size(); i++) {
        size_t j = 0;
        while (j < prefix.size() && j < matches[i].size() &&
               prefix[j] == matches[i][j]) {
            j++;
        }
        prefix = prefix.substr(0, j);
        if (prefix.empty()) break;
    }
    return prefix;
}

string autocomplete(const string &current_input, bool list_all) {
    vector<string> tokens;
    string token;
    istringstream iss(current_input);
    while (iss >> token) tokens.push_back(token);

    string last_token = tokens.empty() ? "" : tokens.back();
    vector<string> matches;

    if (tokens.size() <= 1) {
        vector<string> all_cmds = internal_cmds;
        vector<string> path_cmds = get_path_commands();
        all_cmds.insert(all_cmds.end(), path_cmds.begin(), path_cmds.end());
        sort(all_cmds.begin(), all_cmds.end());
        for (auto &cmd : all_cmds) {
            if (cmd.find(last_token) == 0) matches.push_back(cmd);
        }
    } else {
        matches = list_files(last_token);
    }

    if (matches.empty()) return current_input;

    if (matches.size() == 1) {
        string completion = matches[0];
        string result = current_input.substr(0, current_input.size() - last_token.size());
        return result + completion + " ";
    }

    string prefix = common_prefix(matches);
    if (prefix.size() > last_token.size()) {
        string result = current_input.substr(0, current_input.size() - last_token.size());
        return result + prefix;
    }

    if (list_all) {
        cout << "\n";
        for (auto &m : matches) cout << m << "  ";
        cout << "\n" << current_input;
        cout.flush();
    }
    return current_input;
}
