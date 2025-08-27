#pragma once
#include <iostream>

struct Message
{
    int tag;
    int length;
    std::string value;
};

class ProtocolParser
{
public:
    ProtocolParser() {}
    ~ProtocolParser() {}

    // 从字节流解析为 Message
    static bool Parse(const std::string &raw,Message &msg);
    // 将Message序列化为字节流
    static std::string serialize(const Message &msg);
};
