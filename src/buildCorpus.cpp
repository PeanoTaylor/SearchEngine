#include "PageProcessor.hpp"
#include "KeywordProcessor.hpp"
#include "DirectoryScanner.hpp"
#include <iostream>
using namespace std;

int main()
{
    cout << "开始构建语料索引..." << endl;
    PageProcessor pp;
    pp.process("../corpus/webpages"); // 语料目录

    cout << "开始构建词典索引..." << endl;
    KeyWordProcessor kwp;
    kwp.process("../corpus/CN", "../corpus/EN");

    cout << "离线构建完成！" << endl;
    return 0;
}
