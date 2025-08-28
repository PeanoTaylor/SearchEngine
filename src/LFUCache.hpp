#pragma once
#include <unordered_map>
#include <list>
#include <string>
#include <mutex>
#include <vector>
using std::list;
using std::lock_guard;
using std::mutex;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

class LFUCache
{
public:
    explicit LFUCache(size_t capacity) : _capacity(capacity), _minFreq(0) {}

    bool get(const string &key, string &value)
    {
        lock_guard<mutex> lg(_mutex);
        auto it = _map.find(key);
        if (it == _map.end())
            return false;

        boostFreq(it->first, it->second); // 增加频率
        value = it->second.value;
        return true;
    }

    void put(const string &key, const string &value)
    {
        lock_guard<mutex> lg(_mutex);
        auto it = _map.find(key);
        if (it != _map.end())
        {
            // key存在，更新值并增加频率
            it->second.value = value;
            boostFreq(it->first, it->second);
            return;
        }
        // 超出容量,淘汰尾部节点
        if (_map.size() >= _capacity)
        {
            auto &list = _freq[_minFreq];
            auto del = list.back();
            list.pop_back(); // 链表尾部的key是最早加入、最久没被访问的

            if (list.empty())
                _freq.erase(_minFreq);
            _map.erase(del);
        }

        // 插入新节点，freq=1
        _freq[1].push_front(key);
        _map[key] = {value, 1, _freq[1].begin()};
        _minFreq = 1;
    }

    // 导出所有的key-value,同步共享缓存
    vector<pair<string, string>> exportData()
    {
        lock_guard<mutex> lg(_mutex);
        vector<pair<string, string>> res;
        for (auto &kv : _map)
        {
            res.push_back({kv.first, kv.second.value});
        }
        return res;
    }

    void clear()
    {
        lock_guard<mutex> lg(_mutex);
        _map.clear();
        _freq.clear();
        _minFreq = 0;
    }
    ~LFUCache() {}

private:
    struct Node
    {
        string value;              // 存储的值
        int freq;                  // 访问频率
        list<string>::iterator it; // 指向该 key 在 _freq[freq] 链表中的位置
    };
    void boostFreq(const string &key, Node &node)
    {
        // 找到需要增加频率节点，从当前链表中删除
        int freq = node.freq;
        auto &list = _freq[freq];
        list.erase(node.it);

        if (list.empty())
        {
            _freq.erase(freq);    // 移除当前频率组
            if (_minFreq == freq) // 当前全局最小频率为这组中元素
                _minFreq++;
        }
        // 插入到 freq+1 的链表头
        _freq[freq + 1].push_front(key);   // 把 key 从频率=freq的组，提升到频率=freq+1的组
        node.freq++;                       // 提升结点频率
        node.it = _freq[freq + 1].begin(); // 更新 node 的频率和迭代器，保证 _map 和 _freq 的数据一致性。
    }

private:
    size_t _capacity;
    int _minFreq;
    unordered_map<string, Node> _map;       // 保存当前缓存中所有key
    unordered_map<int, list<string>> _freq; // 频率 → 双向链表(key 集合)，list存储了所有频率等于 _minFreq 的 key(拉链法)
    mutex _mutex;
};
