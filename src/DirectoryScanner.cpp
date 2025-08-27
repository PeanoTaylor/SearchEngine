#include "DirectoryScanner.hpp"
#include <dirent.h>
#include <sys/types.h>
#include <iostream>
using namespace std;

// 遍历目录
vector<string> DirectoryScanner::scan(const string& dir){
    vector<string> files;
    DIR *dp = opendir(dir.c_str());
    // 判断目录是否在存在
    if(!dp){
        perror("fail open");
        return files;
    }

    struct dirent* entry;
    while((entry = readdir(dp)) != nullptr){
        string name(entry->d_name);
        if(name == "." || name == "..")
            continue;
        files.emplace_back(dir + "/" +name);
    }
    closedir(dp);
    return files;
}
