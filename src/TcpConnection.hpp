/**
 * Project 66th
 */

#pragma once
#include "SocketIO.hpp"
#include "Socket.hpp"
#include "InetAddress.hpp"
#include "EventLoop.hpp"
#include <string>
#include <sstream>
#include <memory>
#include <functional>
using std::function;
using std::ostringstream;
using std::shared_ptr;
using std::string;
class TcpConnection;
class EventLoop;
using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallBack = function<void(const TcpConnectionPtr &)>;
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    /**
     * @param fd
     * @param loop
     */
    TcpConnection(int fd, EventLoop *loop);

    ~TcpConnection();

    bool isClosed();

    string receive();

    /**
     * @param msg
     */
    void send(const string &msg);

    /**
     * @param msg
     */
    void sendInLoop(const string &msg);

    string toString();

    /**
     * @param cb
     */
    void setNewConnectionCallback(const TcpConnectionCallBack &cb);

    /**
     * @param cb
     */
    void setMessageCallback(const TcpConnectionCallBack &cb);

    /**
     * @param cb
     */
    void setCloseCallback(const TcpConnectionCallBack &cb);

    void handleNewConnectionCallback();

    void handleMessageCallback();

    void handleCloseCallback();

private:
    EventLoop *_loop;
    SocketIO _sockIO;
    Socket _sock;
    InetAddress _localAddr;
    InetAddress _peerAddr;
    TcpConnectionCallBack _onNewConnection;
    TcpConnectionCallBack _onMessage;
    TcpConnectionCallBack _onClose;

    InetAddress getLocalAddr();

    InetAddress getPeerAddr();
};

