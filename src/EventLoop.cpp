/**
 * Project 66th
 */

#include "EventLoop.hpp"
#include "Logger.hpp"

/**
 * EventLoop implementation
 */

/**
 * @param acc
 */
EventLoop::EventLoop(Acceptor &acc) : _epfd(createEpollFd()), _evtfd(createEventFd()), _isLooping(false), _acceptor(acc), _evlist(OPEN_MAX)
{
    addEpollReadFd(_acceptor.fd());
    addEpollReadFd(_evtfd);
}

EventLoop::~EventLoop()
{
}

/**
 * @return void
 */
void EventLoop::loop()
{
    _isLooping = true;
    while (_isLooping)
    {
        waitEpollFd();
    }
}

/**
 * @return void
 */
void EventLoop::unloop()
{
    _isLooping = false;
}

/**
 * @param cb
 */
void EventLoop::setNewConnectionCallBack(TcpConnectionCallBack &&cb)
{
    _onNewConnection = std::move(cb);
}

/**
 * @param cb
 */
void EventLoop::setMessageCallBack(TcpConnectionCallBack &&cb)
{
    _onMessage = std::move(cb);
}

/**
 * @param cb
 */
void EventLoop::setCloseCallBack(TcpConnectionCallBack &&cb)
{
    _onClose = std::move(cb);
}

/**
 * @return int
 */

/**
 * @return void
 */
void EventLoop::handleRead()
{
    uint64_t flag = 1;
    read(_evtfd, &flag, sizeof(flag));
    doPendingFunctors();
}

/**
 * @return void
 */
void EventLoop::wakeup()
{
    uint64_t flag = 1;
    write(_evtfd, &flag, sizeof(flag)); // 唤醒epoll_wait
}

/**
 * @param cb
 * @return void
 */
void EventLoop::runInLoop(function<void()> cb)
{
    {
        lock_guard<mutex> lg(_mutex);
        _pendingFunctors.push_back(std::move(cb));
    }
    wakeup();
}

/**
 * @return void
 */
void EventLoop::doPendingFunctors()
{
    vector<function<void()>> functors;
    {
        lock_guard<mutex> lg(_mutex);
        functors.swap(_pendingFunctors);
    }
    for (auto &f : functors)
        f();
}

int EventLoop::createEventFd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK);
    if (evtfd < 0)
    {
        LOG_ERROR("eventfd create failed");
        exit(EXIT_FAILURE);
    }
    return evtfd;
}
/**
 * @return int
 */
int EventLoop::createEpollFd()
{
    _epfd = epoll_create(OPEN_MAX);
    if (_epfd < 0)
    {
        LOG_ERROR("epoll create failed");
        exit(EXIT_FAILURE);
    }
    return _epfd;
}

/**
 * @param fd
 * @return void
 */
void EventLoop::addEpollReadFd(int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &event);
    if (ret < 0)
    {
        LOG_ERROR("epoll add failed");
        exit(EXIT_FAILURE);
    }
}

/**
 * @param fd
 * @return void
 */
void EventLoop::delEpollReadFd(int fd)
{
    int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
    if (ret < 0)
    {
        LOG_ERROR("epoll del failed");
        exit(EXIT_FAILURE);
    }
}

/**
 * @return void
 */
void EventLoop::waitEpollFd()
{
    struct epoll_event ep[OPEN_MAX];
    int nready = epoll_wait(_epfd, ep, OPEN_MAX, -1);
    if (nready < 0)
    {
        LOG_ERROR("epoll wait failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < nready; ++i)
    {
        if (ep[i].data.fd == _acceptor.fd())
        {
            handleNewConnection();
        }
        else if (ep[i].data.fd == _evtfd)
        {
            handleRead();
        }
        else
        {
            handleMessage(ep[i].data.fd);
        }
    }
}
/**
 * @return void
 */
void EventLoop::handleNewConnection()
{
    int connfd = _acceptor.accept();
    if (connfd < 0)
    {
        LOG_ERROR("handleNewConnection failed");
        exit(EXIT_FAILURE);
    }
    LOG_INFO(("Accepted new connection fd=" + std::to_string(connfd)).c_str());

    auto conn = make_shared<TcpConnection>(connfd, this);
    // 注册回调
    conn->setNewConnectionCallback(_onNewConnection);
    conn->setMessageCallback(_onMessage);
    conn->setCloseCallback(_onClose);

    _conns[connfd] = conn;
    addEpollReadFd(connfd);

    conn->handleNewConnectionCallback();
}

/**
 * @param fd
 * @return void
 */
void EventLoop::handleMessage(int fd)
{
    auto it = _conns.find(fd);
    if (it != _conns.end())
    {
        auto conn = it->second;
        bool flag = conn->isClosed();
        if (flag)
        {
            if (_onClose)
            {
                delEpollReadFd(fd);
                _conns.erase(it);
                close(fd);
                conn->handleCloseCallback();
            }
            LOG_INFO(("connection closed fd=" + std::to_string(fd)).c_str());
        }
        else
        {
            conn->handleMessageCallback();
            LOG_DEBUG(("message from fd=" + std::to_string(fd)).c_str());
        }
    }
}