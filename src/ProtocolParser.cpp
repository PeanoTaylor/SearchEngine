#include "ProtocolParser.hpp"
#include <iostream>
#include <cstring>
using namespace std;

// ProtocolParser.cpp
bool ProtocolParser::Parse(const std::string &raw, Message &msg)
{
    if (raw.size() < 2 * sizeof(int))
    {
        return false; // tag + length
    }

    const char *p = raw.data();
    int tag = 0, length = 0;

    // 用 memcpy 避免未对齐的问题
    std::memcpy(&tag, p, sizeof(int));
    p += sizeof(tag);// 
    std::memcpy(&length, p, sizeof(int));
    p += sizeof(length);

    if (length < 0)
        return false;
    if (raw.size() < 2 * sizeof(int) + length)
    {
        return false; // 包体不完整
    }

    msg.tag = tag;
    msg.length = length;
    msg.value.assign(p, length);
    return true; // ✅ 别忘了
}

std::string ProtocolParser::serialize(const Message &msg)
{
    string raw;
    raw.reserve(msg.length + 2 * sizeof(int)); // 预留空间

    // 消息写入内存
    raw.append(reinterpret_cast<const char *>(&msg.tag), sizeof(int));
    raw.append(reinterpret_cast<const char *>(&msg.length), sizeof(int));
    raw.append(msg.value);

    return raw;
}