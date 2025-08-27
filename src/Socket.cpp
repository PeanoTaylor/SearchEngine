/**
 * Project 66th
 */

#include "Socket.hpp"

/**
 * Socket implementation
 */

Socket::Socket()
{
    _fd = socket(AF_INET,SOCK_STREAM,0);
    if(_fd < 0){
        perror("socket");
        return;
    }
}

/**
 * @param fd
 */
Socket::Socket(int fd)
{
    _fd = fd;
}

Socket::~Socket()
{
}

/**
 * @return int
 */
int Socket::getFd()
{
    return _fd;
}