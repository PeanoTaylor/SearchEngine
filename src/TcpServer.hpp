/**
 * Project 66th
 */


#pragma once

#include "Acceptor.hpp"
#include "EventLoop.hpp"


class TcpServer {
public: 
    using callback = function<void(const TcpConnectionPtr &)>;
    /**
     * @param ip
     * @param port
     */
    TcpServer(const string & ip, unsigned short port);

    ~TcpServer();

    void start();

    void stop();

    /**
     * @param cb1
     * @param cb2
     * @param cb3
     */
    void setAllCallback(callback && cb1, callback && cb2, callback &&cb3);
private: 
    Acceptor _acceptor;
    EventLoop _loop;
};


