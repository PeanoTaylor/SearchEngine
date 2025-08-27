/**
 * Project 66th
 */

#include "TcpConnection.hpp"
#include "Logger.hpp"
#include <cstring>
#include <iostream>

/**
 * TcpConnection implementation
 */

/**
 * @param fd
 * @param loop
 */
TcpConnection::TcpConnection(int fd, EventLoop *loop)
    : _sock(fd), _sockIO(fd), _localAddr(getLocalAddr()), _peerAddr(getPeerAddr()), _loop(loop)
{
}

TcpConnection::~TcpConnection()
{
}

/**
 * @return bool
 */
// 进行一次复制式的预读取
// 如果读到的字符数为0，代表连接已经断开了
bool TcpConnection::isClosed()
{
    char buf[5] = {0};
    int ret = ::recv(_sock.getFd(), buf, sizeof(buf), MSG_PEEK);
    if (ret < 0)
    {
        LOG_ERROR("recv(MSG_PEEK) failed");
    }
    return 0 == ret;
}

/**
 * @return string
 */
string TcpConnection::receive()
{
    // 先读 8 字节头
    int headSize = sizeof(int) * 2;
    char header[8] = {0};
    int n = _sockIO.readn(header, headSize);
    if (n == 0)
    {
        return ""; // 对端关闭
    }
    else if (n < headSize)
    {
        return ""; // 不完整
    }

    int tag = 0;
    int length = 0;
    memcpy(&tag, header, 4);
    memcpy(&length, header + 4, 4);

    if (length < 0 || length > 10 * 1024 * 1024)
    {
        LOG_ERROR(("Invalid packet length: " + std::to_string(length)).c_str());
        return "";
    }

    string value(length, '\0');
    n = _sockIO.readn(&value[0], length);
    if (n < length)
    {
        LOG_WARN("Incomplete packet received");
        return "";
    }

    // 返回完整二进制包（头+体）
    string packet;
    packet.append(header, headSize);
    packet.append(value);
    LOG_DEBUG(("Received packet length=" + std::to_string(length)).c_str());
    return packet;
}

/**
 * @param msg
 * @return void
 */
void TcpConnection::send(const string &msg)
{
    int n = _sockIO.writen(const_cast<char *>(msg.c_str()), msg.size());
    if (n < (int)msg.size())
    {
        LOG_ERROR("send failed or partial write");
    }
    else
    {
        LOG_DEBUG(("send success size=" + std::to_string(msg.size())).c_str());
    }
}
/**
 * @param msg
 * @return void
 */
void TcpConnection::sendInLoop(const string &msg)
{
    auto con = shared_from_this();
    _loop->runInLoop([con, msg]
                     { con->send(msg); });
}

/**
 * @return string
 */
string TcpConnection::toString()
{
    ostringstream oss;
    oss << _localAddr.getIp() << ":" << _localAddr.getPort() << "->" << _peerAddr.getIp() << ":" << _peerAddr.getPort();
    return oss.str();
}

/**
 * @param cb
 */
void TcpConnection::setNewConnectionCallback(const TcpConnectionCallBack &cb)
{
    _onNewConnection = cb;
}

/**
 * @param cb
 */
void TcpConnection::setMessageCallback(const TcpConnectionCallBack &cb)
{
    _onMessage = cb;
}

/**
 * @param cb
 */
void TcpConnection::setCloseCallback(const TcpConnectionCallBack &cb)
{
    _onClose = cb;
}

/**
 * @return void
 */
void TcpConnection::handleNewConnectionCallback()
{
    if (_onNewConnection)
        _onNewConnection(shared_from_this());
}

/**
 * @return void
 */
void TcpConnection::handleMessageCallback()
{
    if (_onMessage)
        _onMessage(shared_from_this());
}

/**
 * @return void
 */
void TcpConnection::handleCloseCallback()
{
    if (_onClose)
        _onClose(shared_from_this());
}

/**
 * @return InetAddress
 */
InetAddress TcpConnection::getLocalAddr()
{
    struct sockaddr_in localaddr;
    socklen_t addr_len = sizeof(localaddr);
    int ret = getsockname(_sock.getFd(), (struct sockaddr *)&localaddr, &addr_len);
    if (ret < 0)
    {
        perror("getsockname");
    }
    return InetAddress(localaddr);
}

/**
 * @return InetAddress
 */
InetAddress TcpConnection::getPeerAddr()
{
    struct sockaddr_in peeraddr;
    socklen_t addr_len = sizeof(peeraddr);
    int ret = getpeername(_sock.getFd(), (struct sockaddr *)&peeraddr, &addr_len);
    if (ret < 0)
    {
        perror("getpeername");
    }
    return InetAddress(peeraddr);
}