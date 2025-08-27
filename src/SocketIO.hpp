/**
 * Project 66th
 */

#pragma once
#include <string>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
using std::string;
class SocketIO
{
public:
    /**
     * @param fd
     */
    explicit SocketIO(int fd);

    ~SocketIO();

    /**
     * @param buf
     * @param len
     */
    int readn(char *buf, int len);

    /**
     * @param buf
     * @param len
     */
    int readLine(char *buf, int len);

    /**
     * @param buf
     * @param len
     */
    int writen(char *buf, int len);

private:
    int _fd;
};
