#include "WebPageSearcher.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

// 构造函数：加载索引与偏移表
WebPageSearcher::WebPageSearcher(const std::string &pagesFile, const std::string &offsetsFile, const std::string &indexFile)
    : _pagesFile(pagesFile)
{
    loadOffsets(offsetsFile);
    loadInvertIndex(indexFile);
}

// 加载偏移库
void WebPageSearcher::loadOffsets(const std::string &offsetsFile)
{
    std::ifstream ifs(offsetsFile);
    if (!ifs.is_open())
    {
        std::cerr << "Error: cannot open offsets file " << offsetsFile << std::endl;
        return;
    }

    int id;
    size_t offset, len;
    while (ifs >> id >> offset >> len)
    {
        _offsetTable[id] = {id, offset, len};
    }
}

// 加载倒排索引库
void WebPageSearcher::loadInvertIndex(const std::string &indexFile)
{
    std::ifstream ifs(indexFile);
    if (!ifs.is_open())
    {
        std::cerr << "Error: cannot open invert index file " << indexFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(ifs, line))
    {
        std::istringstream iss(line);
        std::string word;
        iss >> word;
        int docId;
        double weight;
        while (iss >> docId >> weight)
        {
            _invertIdx[word][docId] = weight;
        }
    }
}

// 简单分词（英文按空格，中文逐字）
std::vector<std::string> WebPageSearcher::tokenize(const std::string &text)
{
    std::vector<std::string> res;
    bool isAscii = true;
    for (unsigned char c : text)
    {
        if (c & 0x80)
        {
            isAscii = false;
            break;
        }
    }

    if (isAscii)
    {
        std::istringstream iss(text);
        std::string token;
        while (iss >> token)
        {
            for (auto &ch : token)
                ch = std::tolower(ch);
            res.push_back(token);
        }
    }
    else
    {
        for (size_t i = 0; i < text.size();)
        {
            unsigned char c = text[i];
            int len = 1;
            if ((c & 0xF0) == 0xE0)
                len = 3; // 中文常用
            else if ((c & 0xE0) == 0xC0)
                len = 2;
            else if ((c & 0xF8) == 0xF0)
                len = 4;
            if (i + len > text.size())
                break;
            res.push_back(text.substr(i, len));
            i += len;
        }
    }
    return res;
}

// UTF-8 安全截断（避免截半个汉字）
std::string WebPageSearcher::safeUtf8Substr(const std::string &str, size_t maxChars)
{
    std::string result;
    size_t i = 0, count = 0;
    while (i < str.size() && count < maxChars)
    {
        unsigned char c = str[i];
        size_t charLen = 1;
        if ((c & 0x80) == 0)
            charLen = 1;
        else if ((c & 0xE0) == 0xC0)
            charLen = 2;
        else if ((c & 0xF0) == 0xE0)
            charLen = 3;
        else if ((c & 0xF8) == 0xF0)
            charLen = 4;
        if (i + charLen > str.size())
            break; // 防止越界
        result.append(str.substr(i, charLen));
        i += charLen;
        count++;
    }
    return result;
}

// 根据 docId 获取网页片段
std::string WebPageSearcher::getDocSnippet(int docId)
{
    auto it = _offsetTable.find(docId);
    if (it == _offsetTable.end())
        return "";

    std::ifstream ifs(_pagesFile);
    if (!ifs.is_open())
        return "";

    ifs.seekg(it->second.offset);
    std::string buf(it->second.length, '\0');
    ifs.read(&buf[0], it->second.length);

    std::string title, content;
    size_t t1 = buf.find("<title>");
    size_t t2 = buf.find("</title>");
    if (t1 != std::string::npos && t2 != std::string::npos)
    {
        title = buf.substr(t1 + 7, t2 - (t1 + 7));
    }
    size_t c1 = buf.find("<content>");
    size_t c2 = buf.find("</content>");
    if (c1 != std::string::npos && c2 != std::string::npos)
    {
        std::string rawContent = buf.substr(c1 + 9, c2 - (c1 + 9));
        content = safeUtf8Substr(rawContent, 100); // 按字符截断
    }
    return title + " : " + content;
}

// 执行查询
std::string WebPageSearcher::doQuery(const std::string &query)
{
    auto words = tokenize(query);
    std::unordered_map<int, double> docScores;

    for (auto &w : words)
    {
        if (_invertIdx.count(w))
        {
            for (auto &[docId, weight] : _invertIdx[w])
            {
                docScores[docId] += weight;
            }
        }
    }

    std::vector<std::pair<int, double>> results(docScores.begin(), docScores.end());
    std::sort(results.begin(), results.end(),
              [](auto &a, auto &b)
              { return a.second > b.second; });

    nlohmann::json j;
    j["query"] = query;
    j["results"] = nlohmann::json::array();

    for (int i = 0; i < std::min(5, (int)results.size()); ++i)
    {
        int docId = results[i].first;
        j["results"].push_back({{"docId", docId},
                                {"score", results[i].second},
                                {"snippet", getDocSnippet(docId)}});
    }

    if (results.empty())
    {
        j["results"] = nlohmann::json::array();
        j["msg"] = "未找到相关结果";
    }
    return j.dump(4);
}
