#pragma once
#include <vector>
#include <string>
using std::vector;
using std::string;

class DirectoryScanner
{
public:
    /**
     * 遍历目录 dir, 获取目录里面的所有文件名
     */ 
    static vector<string> scan(const string& dir);

private:
    DirectoryScanner() = delete;
};
