#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>

// 网页搜索器
class WebPageSearcher {
public:
    WebPageSearcher(const std::string& pagesFile,
                    const std::string& offsetsFile,
                    const std::string& indexFile);

    // 执行查询，返回 JSON 结果
    std::string doQuery(const std::string& query);

private:
    struct Offset {
        int docId;
        size_t offset;
        size_t length;
    };

    // 加载数据
    void loadOffsets(const std::string& offsetsFile);
    void loadInvertIndex(const std::string& indexFile);

    // 工具
    std::vector<std::string> tokenize(const std::string& text);
    std::string getDocSnippet(int docId);

    // 安全 UTF-8 截断
    static std::string safeUtf8Substr(const std::string &str, size_t maxChars);

    // 数据结构
    std::unordered_map<int, Offset> _offsetTable;                       // 偏移表
    std::unordered_map<std::string, std::map<int, double>> _invertIdx;  // 倒排索引
    std::string _pagesFile;                                             // 网页库路径
};
