#include "KeyRecommander.hpp"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <set>
#include <queue>
#include <string>
#include <nlohmann/json.hpp>

using std::string;
using json = nlohmann::json;
// 候选词
struct CandidateWords
{
    string word;
    int editDist;
    int freq;
    bool operator<(const CandidateWords &rhs) const
    {
        if (editDist != rhs.editDist)
            return editDist > rhs.editDist; // 编辑距离小优先
        if (freq != rhs.freq)
            return freq < rhs.freq; // 词频大优先
        return word > rhs.word;     // 字典序小优先
    }
};

KeyRecommander::KeyRecommander(const string &enDictFile, const string &cnDictFile, const string &enIndexFile, const string &cnIndexDictFile)
{
    loadDictFile(enDictFile, cnDictFile);
    loadIndexFile(enIndexFile, cnIndexDictFile);
}

// 英文查询
string KeyRecommander::doQueryEn(const string &word, int k)
{
    std::set<int> candidateIds; // 计算词汇所含字符出现的位置(dict行号)
    for (char ch : word)
    {
        string key(1, ch); // 构造为string -> unordered_map.first
        if (en_index.count(key))
        {                                                                    // 若索引库中存在该字符
            candidateIds.insert(en_index[key].begin(), en_index[key].end()); // 记录该字符出现的记录(字典行号)
        }
    }

    std::priority_queue<CandidateWords> pq; // 优先队列，根据优先级大小排序
    for (int id : candidateIds)
    {
        const auto &candidate = en_dict[id].first;
        int dist = editDistance(word, candidate);
        int freq = en_dict[id].second;
        pq.push({candidate, dist, freq});
    }

    json result;
    result["candidates"] = json::array();
    for (int i = 0; i < k && !pq.empty(); ++i)
    {
        auto top = pq.top();
        pq.pop();
        result["candidates"].push_back({{"word", top.word}, {"freq", top.freq}, {"dist", top.editDist}});
    }

    return result.dump(4);
}

// 🔹 UTF-8 分割（中文用）
static std::vector<string> splitUtf8(const string &s)
{
    std::vector<string> res;
    for (size_t i = 0; i < s.size();)
    {
        unsigned char c = s[i];
        int len = 1;
        if ((c & 0x80) == 0)
            len = 1;
        else if ((c & 0xE0) == 0xC0)
            len = 2;
        else if ((c & 0xF0) == 0xE0)
            len = 3; // 中文常用
        else if ((c & 0xF8) == 0xF0)
            len = 4;
        res.push_back(s.substr(i, len));
        i += len;
    }
    return res;
}

// 中文查询
string KeyRecommander::doQueryCn(const string &word, int k)
{
    auto chars = splitUtf8(word);

    std::set<int> candidateIds;
    for (auto &ch : chars)
    {
        if (cn_index.count(ch))
        {
            candidateIds.insert(cn_index[ch].begin(), cn_index[ch].end());
        }
    }

    std::priority_queue<CandidateWords> pq;
    for (int id : candidateIds)
    {
        const auto &cand = cn_dict[id].first;
        int dist = editDistance(word, cand);
        int freq = cn_dict[id].second;
        pq.push({cand, dist, freq});
    }

    json result;
    result["candidates"] = json::array();
    for (int i = 0; i < k && !pq.empty(); ++i)
    {
        auto top = pq.top();
        pq.pop();
        result["candidates"].push_back({{"word", top.word},
                                        {"freq", top.freq}});
    }
    return result.dump(4);
}

// 计算编辑距离
int KeyRecommander::editDistance(const std::string &lhs, const std::string &rhs)
{
    size_t m = lhs.size(), n = rhs.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
    for (size_t i = 0; i <= m; ++i)
        dp[i][0] = i;
    for (size_t j = 0; j <= n; ++j)
        dp[0][j] = j;

    for (size_t i = 1; i <= m; ++i)
    {
        for (size_t j = 1; j <= n; ++j)
        {
            if (lhs[i - 1] == rhs[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + 1});
        }
    }
    return dp[m][n];
}

// 加载字典
void KeyRecommander::loadDictFile(const string &enDictFile, const string &cnDictFile)
{
    std::ifstream ifs_en(enDictFile);
    if (!ifs_en.is_open())
    {
        std::cerr << "Failed to open English dict file: " << enDictFile << std::endl;
        return;
    }
    string word;
    int freq;
    while (ifs_en >> word >> freq)
    {
        en_dict.emplace_back(word, freq);
    }

    std::ifstream ifs_ch(cnDictFile);
    if (!ifs_ch.is_open())
    {
        std::cerr << "Failed to open Chinese dict file: " << cnDictFile << std::endl;
        return;
    }
    while (ifs_ch >> word >> freq)
    {
        cn_dict.emplace_back(word, freq);
    }
}

// 加载索引库
void KeyRecommander::loadIndexFile(const string &enIndexFile, const string &cnIndexDictFile)
{
    std::ifstream ifs_en(enIndexFile);
    if (!ifs_en.is_open())
    {
        std::cerr << "Failed to open English index file: " << enIndexFile << std::endl;
        return;
    }

    string key;
    int idx;
    string line;
    while (std::getline(ifs_en, line))
    {
        std::istringstream iss(line);
        if (!(iss >> key))
            continue;
        while (iss >> idx)
        {
            en_index[key].push_back(idx);
        }
    }

    std::ifstream ifs_ch(cnIndexDictFile);
    if (!ifs_ch.is_open())
    {
        std::cerr << "Failed to open Chinese index file: " << cnIndexDictFile << std::endl;
        return;
    }
    while (std::getline(ifs_ch, line))
    {
        std::istringstream iss(line);
        if (!(iss >> key))
            continue;
        while (iss >> idx)
        {
            cn_index[key].push_back(idx);
        }
    }
}
