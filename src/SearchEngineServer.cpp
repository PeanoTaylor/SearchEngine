#include "SearchEngineServer.hpp"
#include "Logger.hpp"
#include "ProtocolParser.hpp"
#include "CacheManage.hpp"
#include <functional>
#include <iostream>
using namespace std;

// 全局静态缓存管理器（容量1000，可配置化）
static CacheManage cache(100, "tcp://127.0.0.1:6379");


SearchEngineServer::SearchEngineServer(const std::string &ip, int port)
    : _tcpserver(ip, port), _threadpool(4, 10) {}

void SearchEngineServer::start()
{
    _threadpool.start();
    // 注册三个回调函数
    _tcpserver.setAllCallback(
        std::bind(&SearchEngineServer::onConnection, this, std::placeholders::_1),
        std::bind(&SearchEngineServer::onMessage, this, std::placeholders::_1),
        std::bind(&SearchEngineServer::onClose, this, std::placeholders::_1));

    LOG_INFO("SearchEngineServer start ...");
    _tcpserver.start();
}

void SearchEngineServer::stop()
{
    _threadpool.stop();
    _tcpserver.stop();
    LOG_INFO("SearchEngineServer stop ...");
}

void SearchEngineServer::onConnection(const TcpConnectionPtr &conn)
{
    string msg = conn->toString() + " has connected.";
    LOG_INFO(msg.c_str());
}

// 任务到达，让线程池来执行任务
void SearchEngineServer::onMessage(const TcpConnectionPtr &conn)
{
    string msg = conn->receive(); // 已经是完整包
    if (!msg.empty())
    {
        LOG_INFO(("received msg size=" + to_string(msg.size())).c_str());
        _threadpool.addTask(std::bind(&SearchEngineServer::doTaskThread, this, conn, msg));
    }
    else
    {
        LOG_INFO("peer closed connection");
    }
}

void SearchEngineServer::onClose(const TcpConnectionPtr &conn)
{
    string msg = conn->toString() + " has closed.";
    LOG_INFO(msg.c_str());
}

void SearchEngineServer::doTaskThread(const TcpConnectionPtr &conn, std::string &msg)
{
    // 这里可以使用 ProtocolParser 解包
     Message message;
    if (!ProtocolParser::Parse(msg, message))
    {
        LOG_WARN("Protocol parse error");
        conn->sendInLoop("{\"error\":\"Parse error\"}\n");
        return;
    }

    // 拼接缓存 key: "tag:value"
    string cacheKey = std::to_string(message.tag) + ":" + message.value;
    string result = cache.get(cacheKey);

    conn->sendInLoop(result + "\n");
    LOG_INFO("Query result sent to client");
}
