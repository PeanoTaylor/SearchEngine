#include "PageProcessor.hpp"
#include "simhash/Simhasher.hpp"
#include "DirectoryScanner.hpp"
#include <tinyxml2.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <sstream>
#include <fstream>
#include <cmath>
#include <codecvt>
#include <locale>

using namespace tinyxml2;
using std::ofstream;
using std::ostringstream;
using std::set;
using std::string;
using std::unordered_map;
using std::vector;

// 初始化构造函数
PageProcessor::PageProcessor()
    : m_tokenizer("/usr/local/dict/jieba.dict.utf8",
                  "/usr/local/dict/hmm_model.utf8",
                  "/usr/local/dict/user.dict.utf8",
                  "/usr/local/dict/idf.utf8",
                  "/usr/local/dict/stop_words.utf8"),
      m_hasher("/usr/local/dict/jieba.dict.utf8",
               "/usr/local/dict/hmm_model.utf8",
               "/usr/local/dict/idf.utf8",
               "/usr/local/dict/stop_words.utf8")
{
    std::ifstream ifs("/home/zhang/SearchEngine/corpus/stopwords/cn_stopwords.txt");
    string word;
    while (ifs >> word)
    {
        m_stopWords.insert(word);
    }
}


void PageProcessor::process(const std::string &dir)
{
    extract_documents(dir);
    deduplicate_documents();
    build_pages_and_offsets("../data/webpages.dat", "../data/weboffset.dat");
    build_inverted_index("../data/invertindex.dat");
}

// 提取文档
void PageProcessor::extract_documents(const std::string &dir)
{
    // 获取目录下所有文件路径
    auto files = DirectoryScanner::scan(dir);
    int id = 0;

    for (auto &file : files)
    {
        XMLDocument doc;
        if (doc.LoadFile(file.c_str()) != XML_SUCCESS)
        {
            std::cerr << "Error: cannot load xml file " << file << std::endl;
            continue;
        }

        XMLElement *root = doc.RootElement();
        if (!root)
            continue;

        // 先找 channel
        XMLElement *channel = root->FirstChildElement("channel");
        XMLElement *item = nullptr;
        if (channel)
        {
            item = channel->FirstChildElement("item");
        }
        else
        {
            item = root->FirstChildElement("item"); // 有些语料直接在 root 下
        }

        while (item)
        {
            Document d;
            d.id = id++;

            XMLElement *title = item->FirstChildElement("title");
            XMLElement *link = item->FirstChildElement("link");

            if (title && title->GetText())
                d.title = title->GetText();
            if (link && link->GetText())
                d.link = link->GetText();

            // 如果item中有content
            XMLElement *cont = item->FirstChildElement("content");
            if (cont && cont->GetText())
            {
                d.content = cont->GetText();
            }
            else
            {
                XMLElement *desc = item->FirstChildElement("description");
                if (desc && desc->GetText())
                {
                    d.content = desc->GetText();
                }
            }

            // 4. 只有当 content 非空时，才加入 m_documents
            if (!d.content.empty())
            {
                m_documents.push_back(std::move(d));
            }

            item = item->NextSiblingElement("item");
        }
    }
}

// 计算汉明距离
/* static int hammingDistance(uint64_t hashValue, uint64_t hash)
{
    int xOR = hashValue ^ hash;
    int distance = 0;
    while (xOR)
    {
        distance += (xOR & 1);
        xOR >>= 1;
    }
    return distance;
} */

// 高效汉明距离
static inline int hammingDistance(uint64_t hashValue, uint64_t hash)
{
    return __builtin_popcountll(hashValue ^ hash); // 硬件指令
}

// 文档去重
void PageProcessor::deduplicate_documents()
{
    vector<Document> uniqueDocs;
    set<uint64_t> check_hash;

    for (auto &doc : m_documents)
    {
        uint64_t hashValue = 0;
        // 直接用 make 生成 simhash
        if (!m_hasher.make(doc.content, 200, hashValue))
        {
            std::cerr << "Simhash make() failed for doc: " << doc.id << std::endl;
            continue;
        }

        bool duplicate = false;
        for (auto &hash : check_hash)
        {
            // XOR: 找出不同位
            int dist = hammingDistance(hashValue, hash); // 汉明距离
            if (dist <= 3)                               // 阈值: ≤3 表示相似
            {
                duplicate = true;
                break;
            }
        }

        if (!duplicate)
        {
            check_hash.insert(hashValue); // simhash 记录到集合，方便后续查重
            uniqueDocs.push_back(doc);
        }
    }

    m_documents.swap(uniqueDocs); // 替换掉原来的所有文档
}

// 生成网页库和偏移库
void PageProcessor::build_pages_and_offsets(const string &pagesFile, const string &offsetsFile)
{
    ofstream ofsPages(pagesFile);
    ofstream ofsOffsets(offsetsFile);

    int offset = 0;
    for (auto &doc : m_documents)
    {
        ostringstream oss;
        oss << "<doc>\n"
            << "  <id>" << doc.id << "</id>\n"
            << "  <link>" << doc.link << "</link>\n"
            << "  <title>" << doc.title << "</title>\n"
            << "  <content>" << doc.content << "</content>\n"
            << "</doc>\n";

        string page = oss.str();
        ofsPages << page;

        ofsOffsets << doc.id << " " << offset << " " << page.size() << "\n";
        offset += page.size();
    }
}

// 建立倒排索引
void PageProcessor::build_inverted_index(const std::string &indexFile)
{
    // 计算 TF-IDF
    unordered_map<string, int> df;                             // 包含该关键字的文档个数
    vector<unordered_map<string, int>> tf(m_documents.size()); // 关键字在文档中出现的次数

    for (size_t i = 0; i < m_documents.size(); ++i)
    {
        vector<string> words;                                 // 存分词结果
        m_tokenizer.Cut(m_documents[i].content, words, true); // HMM混合模式

        for (auto &word : words)
        {
            if (m_stopWords.count(word))
                continue;
            ++tf[i][word]; // 第i篇文章中word出现的次数
        }
        for (auto &kv : tf[i]) // 在同一篇文档里多次出现也只算一次 DF
            ++df[kv.first];
    }

    size_t N = m_documents.size();
    for (size_t i = 0; i < N; ++i)
    {
        for (auto &[w, freq] : tf[i])
        {
            double idf = log((double)N / (df[w] + 1));
            double weight = freq * idf;
            m_invertedIndex[w][m_documents[i].id] = weight;
        }
    }

    ofstream ofs(indexFile);
    for (auto &[word, postings] : m_invertedIndex)
    {
        ofs << word << " ";
        for (auto &[docId, weight] : postings)
        {
            ofs << docId << " " << weight << " ";
        }
        ofs << "\n";
    }
}
