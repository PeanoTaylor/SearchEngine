#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using std::string;
using std::unordered_map;
using std::vector;
class KeyRecommander
{
public:
    KeyRecommander(const string &enDictFile, const string &cnDictFile, const string &enIndexFile, const string &cnIndexDictFile);
    ~KeyRecommander() {}
    string doQueryCn(const string &word,int k = 3);
    string doQueryEn(const string &word,int k = 3);

private:
    int editDistance(const string &lhs, const string &rhs);
    void loadDictFile(const string &enDictFile, const string &cnDictFile);
    void loadIndexFile(const string &enIndexFile, const string &cnIndexDictFile);

    // dict: 下标 -> (单词, 词频)
    vector<std::pair<string, int>> cn_dict; // _index中的vector元素能够直接对应 _dict中vector的下表，不用额外建立映射关系，所以不采用map
    vector<std::pair<string, int>> en_dict;
    unordered_map<string, vector<int>> cn_index;
    unordered_map<string, vector<int>> en_index;
};
