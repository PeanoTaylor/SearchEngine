#pragma once
#include "TcpServer.hpp"
#include "ThreadPool.hpp"
#include "CacheManage.hpp"
class SearchEngineServer
{
public:
    SearchEngineServer(const std::string &ip, int port);
    ~SearchEngineServer() = default;
    void start();
    void stop();
    void onConnection(const TcpConnectionPtr &cb_conn);
    void onMessage(const TcpConnectionPtr &cb_mess);
    void onClose(const TcpConnectionPtr &cb_close);
    void doTaskThread(const TcpConnectionPtr &, std::string &msg);

private:
    TcpServer _tcpserver;
    ThreadPool _threadpool;
    CacheManage _cacheManage;
};
