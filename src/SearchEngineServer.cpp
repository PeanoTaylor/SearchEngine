#include "SearchEngineServer.hpp"
#include "Logger.hpp"
#include "ProtocolParser.hpp"
#include <functional>
#include <iostream>
#include <thread>
#include <atomic>

// 线程局部变量，保存每个线程的 MyThread
thread_local std::shared_ptr<MyThread> tls_thread;

SearchEngineServer::SearchEngineServer(const std::string &ip, int port)
    : _tcpserver(ip, port), _threadpool(4, 10), _cacheManage(200, "tcp://127.0.0.1:6379") // 初始化 CacheManage
{
    // 启动缓存同步线程
    _cacheManage.startSyncThread(10);
}

void SearchEngineServer::start()
{
    _threadpool.start();
    // 注册三个回调函数
    _tcpserver.setAllCallback(
        std::bind(&SearchEngineServer::onConnection, this, std::placeholders::_1),
        std::bind(&SearchEngineServer::onMessage, this, std::placeholders::_1),
        std::bind(&SearchEngineServer::onClose, this, std::placeholders::_1));

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
    // 当前线程还没有 MyThread，就创建并注册
    if (!tls_thread)
    {
        static atomic<int> tidGen{0};
        int tid = tidGen++;
        tls_thread = std::make_shared<MyThread>(tid, 100, 50);
        _cacheManage.addThread(tls_thread);

        std::ostringstream oss;
        oss << "Thread init -> tid=" << tid
            << " (cacheCapa=100, patchCapa=50)";
        LOG_INFO(oss.str().c_str());
    }

    // 解包
    Message message;
    if (!ProtocolParser::Parse(msg, message))
    {
        LOG_WARN("Protocol parse error");
        conn->sendInLoop("{\"error\":\"Parse error\"}\n");
        return;
    }

    // 拼接缓存 key: "tag:value"
    string cacheKey = std::to_string(message.tag) + ":" + message.value;

    std::ostringstream oss;
    oss << "Thread tid=" << tls_thread->id
        << " handling query: " << cacheKey;
    LOG_INFO(oss.str().c_str());

    // 使用 CacheManage 查询
    string result = _cacheManage.get(tls_thread.get(), cacheKey);

    conn->sendInLoop(result + "\n");
    oss << "Thread tid=" << tls_thread->id
        << " result sent to client";
    LOG_INFO(oss.str().c_str());
}
