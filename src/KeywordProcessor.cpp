#include "KeywordProcessor.hpp"
#include "DirectoryScanner.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include "utf8cpp/utf8.h"
using std::map;
using std::vector;

// 辅助函数(字符判断)
static bool isAlpha(char c)
{
    return std::isalpha(static_cast<unsigned char>(c));
}

// 初始化jieba
KeyWordProcessor::KeyWordProcessor()
    : m_tokenizer(
          "/usr/local/dict/jieba.dict.utf8", // 词典
          "/usr/local/dict/hmm_model.utf8",  // HMM
          "/usr/local/dict/user.dict.utf8",  // 用户词典
          "/usr/local/dict/idf.utf8",        // idf
          "/usr/local/dict/stop_words.utf8"  // 停用词
      )
{
    std::ifstream ifs_en("/home/zhang/SearchEngine/corpus/stopwords/en_stopwords.txt");
    string word;
    while (ifs_en >> word)
    {   
        m_enStopWords.insert(word);
    }

    std::ifstream ifs_ch("/home/zhang/SearchEngine/corpus/stopwords/cn_stopwords.txt");
    while (ifs_ch >> word)
    {
        m_chStopWords.insert(word);
    }
}

void KeyWordProcessor::process(const string &chDir, const string &enDir)
{
    create_en_dict(enDir, "../data/endict.dat");
    build_en_index("../data/endict.dat", "../data/enindex.dat");

    create_cn_dict(chDir, "../data/cndict.dat");
    build_cn_index("../data/cndict.dat", "../data/cnindex.dat");
}

// 英文部分
// 生成词典库(单词 + 频率)
void KeyWordProcessor::create_en_dict(const string &dir, const string &outfile)
{
    map<string, int> dict;
    auto files = DirectoryScanner::scan(dir);

    for (auto &file : files)
    {
        std::ifstream ifs(file);
        if (!ifs.is_open())
        {
            continue;
        }

        // 去掉数字和标点符号 + 转小写
        string line;
        while (std::getline(ifs, line))
        {

            for (auto &ch : line)
            {
                if (!isAlpha(ch))
                {
                    ch = ' ';
                }
                ch = std::tolower(ch);
            }

            // 统计每个单词(Token)的出现频率
            std::istringstream iss(line);
            string token;
            while (iss >> token)// 空白字符进行切分
            {
                if (m_enStopWords.count(token))
                {
                    continue;
                }
                dict[token]++;// string —> frequency
            }
        }
    }

    // 输出词典库
    std::ofstream ofs(outfile);
    if (!ofs.is_open())
    {
        std::cerr << "Error open file" << std::endl;
        return;
    }
    for (auto &[w, f] : dict)
    {
        ofs << w << " " << f << "\n";
    }
}

// 生成英文索引库
void KeyWordProcessor::build_en_index(const string &dict, const string &index)
{

    std::ifstream ifs(dict);
    if (!ifs.is_open())
    {
        std::cerr << "Error: cannot open dict file " << dict << std::endl;
        return;
    }

    std::map<char, std::vector<int>> charIndex; // 字母 -> 行号集合
    std::vector<std::string> words;             // 保存词典里的单词,通过行号反查单词

    std::string word;
    int freq;
    int lineId = 0;
    while (ifs >> word >> freq)
    {
        words.push_back(word);

        // 遍历单词中的每个字母
        for (char c : word)
        {
            auto &vec = charIndex[c];
            // 避免重复插入
            if (vec.empty() || vec.back() != lineId)
            {
                vec.push_back(lineId);
            }
        }
        lineId++;
    }

    // 写索引文件
    std::ofstream ofs(index);
    if (!ofs.is_open())
    {
        std::cerr << "Error: cannot open index file " << index << std::endl;
        return;
    }

    for (auto &[ch, vec] : charIndex)
    {
        ofs << ch << " ";
        for (int id : vec)
        {
            ofs << id << " ";
        }
        ofs << "\n";
    }
}

// 判断汉字
static bool isChinese(const std::string &s)
{
    auto it = s.begin();
    while (it != s.end())
    {
        char32_t cp = utf8::next(it, s.end());
        if (!((cp >= 0x4E00 && cp <= 0x9FFF) ||   // 基本汉字
              (cp >= 0x3400 && cp <= 0x4DBF) ||   // 扩展A
              (cp >= 0x20000 && cp <= 0x2A6DF) || // 扩展B
              (cp >= 0x2A700 && cp <= 0x2B73F) || // 扩展C
              (cp >= 0x2B740 && cp <= 0x2B81F) || // 扩展D
              (cp >= 0x2B820 && cp <= 0x2CEAF) || // 扩展E
              (cp >= 0x2CEB0 && cp <= 0x2EBEF)    // 扩展F
              ))
        {
            return false; // 只要有一个不是汉字，就判定为 false
        }
    }
    return true;
}

// 中文部分
void KeyWordProcessor::create_cn_dict(const string &dir, const string &outfile)
{
    map<string, int> dict;
    auto files = DirectoryScanner::scan(dir);

    for (auto &file : files)
    {
        std::ifstream ifs(file);
        if (!ifs.is_open())
        {
            continue;
        }

        // 读取整个文件内容
        string content((std::istreambuf_iterator<char>(ifs)),
                       std::istreambuf_iterator<char>());
        // jieba分词
        vector<string> words;
        m_tokenizer.Cut(content, words, true); // true = HMM = 混合模式

        for (auto &w : words)
        {
            if (m_chStopWords.count(w) || !isChinese(w))
                continue; // 过滤停用词或非汉字
            dict[w]++;
        }
    }

    // 输出词典库
    std::ofstream ofs(outfile);
    if (!ofs.is_open())
    {
        std::cerr << "Error open file" << std::endl;
        return;
    }
    for (auto &[w, f] : dict)
    {
        ofs << w << " " << f << "\n";
    }
}

// 生成中文索引库
void KeyWordProcessor::build_cn_index(const string &dict, const string &index)
{
    std::ifstream ifs(dict);
    if (!ifs.is_open())
    {
        std::cerr << "Error: cannot open dict file " << dict << std::endl;
        return;
    }

    map<string, vector<int>> cnindex; // 中文汉字 -> 行号集合
    vector<string> words;             // 保存词典里的单词, 通过行号反查单词

    string word;
    int freq;
    int lineId = 0;

    while (ifs >> word >> freq)
    {
        words.push_back(word);

        // 用 utfcpp 遍历一个个汉字
        for (auto it = word.begin(); it != word.end();)
        {
            char32_t cp = utf8::next(it, word.end()); // 取一个 codepoint 并前进迭代器
            std::string ch;
            utf8::append(cp, std::back_inserter(ch)); // 转回 UTF-8

            cnindex[ch].push_back(lineId); // 建立索引：汉字 -> 行号
        }

        lineId++;
    }

    // 写索引文件
    std::ofstream ofs(index);
    if (!ofs.is_open())
    {
        std::cerr << "Error: cannot open index file " << index << std::endl;
        return;
    }

    for (auto &[ch, vec] : cnindex)
    {
        ofs << ch << " ";
        for (int id : vec)
        {
            ofs << id << " ";
        }
        ofs << "\n";
    }
}
