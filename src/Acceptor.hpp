/**
 * Project 66th
 */

#pragma once
#include <string>
#include "Socket.hpp"
#include "InetAddress.hpp"
using std::string;
class Acceptor
{
public:
    /**
     * @param ip
     * @param port
     */
    Acceptor(const string &ip, unsigned short port);

    ~Acceptor();

    void ready();

    int accept();

    int fd();

private:
    Socket _sock;
    InetAddress _addr;

    void setReuseAddr();

    void setReusePort();

    void bind();

    void listen();
};
