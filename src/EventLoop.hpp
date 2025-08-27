/**
 * Project 66th
 */

#pragma once
#include "Acceptor.hpp"
#include "TcpConnection.hpp"
#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <sys/eventfd.h>
#include <functional>
using std::function;
using std::lock_guard;
using std::make_shared;
using std::map;
using std::mutex;
using std::shared_ptr;
using std::vector;
class TcpConnection;
#define OPEN_MAX 1024
class EventLoop
{
public:
    /**
     * @param acc
     */

    using TcpConnectionPtr = shared_ptr<TcpConnection>;
    using TcpConnectionCallBack = function<void(const TcpConnectionPtr &)>;
    EventLoop(Acceptor &acc);

    ~EventLoop();

    void loop();

    void unloop();

    /**
     * @param cb
     */
    void setNewConnectionCallBack(TcpConnectionCallBack &&cb);

    /**
     * @param cb
     */
    void setMessageCallBack(TcpConnectionCallBack &&cb);

    /**
     * @param cb
     */
    void setCloseCallBack(TcpConnectionCallBack &&cb);

    int createEventFd();

    void handleRead();

    void wakeup();

    /**
     * @param cb
     */
    void runInLoop(function<void()> cb);

    void doPendingFunctors();

private:
    int _epfd;
    vector<struct epoll_event> _evlist;
    bool _isLooping;
    Acceptor &_acceptor;
    map<int, shared_ptr<TcpConnection>> _conns;
    TcpConnectionCallBack _onNewConnection;
    TcpConnectionCallBack _onMessage;
    TcpConnectionCallBack _onClose;
    int _evtfd;
    std::vector<function<void()>> _pendingFunctors;
    mutex _mutex;

    int createEpollFd();

    /**
     * @param fd
     */
    void addEpollReadFd(int fd);

    /**
     * @param fd
     */
    void delEpollReadFd(int fd);

    void waitEpollFd();

    void handleNewConnection();

    /**
     * @param fd
     */
    void handleMessage(int fd);
};
