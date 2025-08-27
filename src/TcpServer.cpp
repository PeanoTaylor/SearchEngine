/**
 * Project 66th
 */

#include "TcpServer.hpp"
#include "EventLoop.hpp"

/**
 * TcpServer implementation
 */

/**
 * @param ip
 * @param port
 */
TcpServer::TcpServer(const string &ip, unsigned short port)
    : _acceptor(ip, port), _loop(_acceptor)
{
}

TcpServer::~TcpServer()
{
}

/**
 * @param cb1
 * @param cb2
 * @param cb3
 * @return void
 */
void TcpServer::setAllCallback(callback &&cb1, callback &&cb2, callback &&cb3)
{
    _loop.setNewConnectionCallBack(std::move(cb1));
    _loop.setMessageCallBack(std::move(cb2));
    _loop.setCloseCallBack(std::move(cb3));
}

/**
 * @return void
 */
void TcpServer::start()
{
    _acceptor.ready();
    _loop.loop();
}

/**
 * @return void
 */
void TcpServer::stop()
{
    _loop.unloop();
}
