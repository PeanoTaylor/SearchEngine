// KeywordProcessor.h
#pragma once
#include <cppjieba/Jieba.hpp>
#include <string>
#include <set>
using std::string;
using std::set;

class KeyWordProcessor {
public:
    KeyWordProcessor();
	
    // chDir: 中文语料库
    // enDir: 英文语料库
    void process(const string& chDir, const string& enDir);

private:
    void create_cn_dict(const string& dir, const string& outfile);
    void build_cn_index(const string& dict, const string& index);

    void create_en_dict(const string& dir, const string& outfile);
    void build_en_index(const string& dict, const string& index);
private:
    cppjieba::Jieba m_tokenizer;
    set<string> m_enStopWords;
    set<string> m_chStopWords;
};