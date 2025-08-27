/**
 * Project 66th
 */

#pragma once
#include <sys/types.h> 
#include <sys/socket.h>
#include <iostream>

class Socket
{
public:
    Socket();

    /**
     * @param fd
     */
    explicit Socket(int fd);

    ~Socket();

    int getFd();

private:
    int _fd;
};
