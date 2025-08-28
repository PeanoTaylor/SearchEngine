#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include <list>
using std::list;
using std::lock_guard;
using std::mutex;
using std::pair;
using std::string;
using std::unordered_map;
// LRUCache: 最近最少使用缓存
// 使用 双向链表 + 哈希表 实现 O(1) 查找和更新
class LRUCache
{
public:
    explicit LRUCache(size_t capacity) : _capacity(capacity) {}

    // 从缓存获取key -> value
    bool get(const string &key, string &value)
    {
        lock_guard<mutex> lg(_mutex);
        auto it = _map.find(key);
        if (it == _map.end())
            return false;

        // 将该节点移动到链表头部(最近使用)
        _list.splice(_list.begin(), _list, it->second);
        value = it->second->second;
        return true;
    }

    void put(const string &key, const string &value)
    {
        lock_guard<mutex> lg(_mutex);
        auto it = _map.find(key);
        if (it != _map.end())
        {
            // key存在，更新值并且移动到链表头部
            it->second->second = value;
            _list.splice(_list.begin(), _list, it->second);
            return;
        }
        // 超出容量,淘汰尾部节点
        if (_list.size() >= _capacity)
        {
            auto last = _list.back();
            _map.erase(last.first);
            _list.pop_back();
        }
        _list.emplace_front(key, value);
        _map[key] = _list.begin();
    }

    ~LRUCache() {}

private:
    size_t _capacity;                                    // 缓存容量
    list<pair<string, string>> _list;                    // (key,value) 链表
    unordered_map<string, decltype(_list.begin())> _map; // 哈希索引
    mutex _mutex;                                        // 线程安全
};
