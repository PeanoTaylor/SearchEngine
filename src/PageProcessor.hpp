#pragma once
#include <string>
#include <vector>
#include <set>
#include "cppjieba/Jieba.hpp"
#include "simhash/Simhasher.hpp"

class PageProcessor
{
public:
    PageProcessor();
    void process(const std::string& dir);

private:
    void extract_documents(const std::string& dir);
    void deduplicate_documents();
    void build_pages_and_offsets(const std::string& pages, const std::string& offsets);
    void build_inverted_index(const std::string& filename);
private:
    struct Document 
    {
        int id;
        std::string link;
        std::string title;
        std::string content;
    };

private:
    cppjieba::Jieba m_tokenizer;
    simhash::Simhasher m_hasher;
    std::set<std::string> m_stopWords;    // 使用set, 而非vector, 是为了方便查找
    std::vector<Document> m_documents;    
    std::map<std::string, std::map<int, double>> m_invertedIndex;
};

