/**
 * Project 66th
 */

#include "SocketIO.hpp"

/**
 * SocketIO implementation
 */

/**
 * @param fd
 */
SocketIO::SocketIO(int fd) : _fd(fd)
{
}

SocketIO::~SocketIO()
{
}

/**
 * @param buf
 * @param len
 * @return int
 */
int SocketIO::readn(char *buf, int len)
{
    int left = len;
    char *pstr = buf;
    while (left > 0)
    {
        int nread = ::recv(_fd, pstr, left, 0);
        if (nread < 0)
        {
            if (errno == EINTR)
                continue;      // 被信号打断重试
            return len - left; // 出错返回已读字节数
        }
        else if (nread == 0)
        {
            return len - left; // 对端关闭
        }
        else
        {
            left -= nread;
            pstr += nread;
        }
    }
    return len;
}

/**
 * @param buf
 * @param len
 * @return int
 */
int SocketIO::readLine(char *buf, int len)
{
    int total = 0, ret = 0;
    int left = len - 1; // 预留1字节放字符串结尾'\0'
    char *pstr = buf;

    while (left > 0)
    {
        int ret = recv(_fd, pstr, left, MSG_PEEK);
        if (ret == 0)
        {
            break; // 对端关闭
        }
        else if (ret == -1 && errno == EINTR)
        {
            continue; // 发生中断
        }
        else // peek到数据，查找有无'\n'
        {
            int idx = 0;
            for (; idx < ret; ++idx)
            {
                if (pstr[idx] == '\n')
                { // 找到了就消费掉socket缓冲区
                    int size = idx + 1;
                    if (readn(pstr, size) != size)
                        return -1;
                    pstr += size;
                    total += size;
                    *pstr = '\0';
                    return total;
                }
            }
            // 没有找到\n,把peek到的全部读出来
            if (readn(pstr, ret) != ret)
                return -1;
            pstr += ret;
            left -= ret;
            total += ret;
        }
    }
    *pstr = '\0';
    return total;
}

/**
 * @param buf
 * @param len
 * @return int
 */
int SocketIO::writen(char *buf, int len)
{
    int left = len;
    char *pstr = buf;
    while (left > 0)
    {
        int nwrite = write(_fd, pstr, left);
        if (nwrite == 0)
        {
            break; // 对端关闭
        }
        else if (nwrite == -1 && errno == EINTR)
        {
            continue; // 发生中断
        }
        else
        {
            left -= nwrite;
            pstr += nwrite;
        }
    }
    return len;
}