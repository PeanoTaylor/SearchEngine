/**
 * Project 66th
 */


#include "InetAddress.hpp"

/**
 * InetAddress implementation
 */


/**
 * @param ip
 * @param port
 */
InetAddress::InetAddress(const string & ip, unsigned short port) {
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr(ip.c_str());
    _addr.sin_port = htons(port);
}

/**
 * @param addr
 */
InetAddress::InetAddress(const struct sockaddr_in & addr) {
    _addr = addr;
}

InetAddress::~InetAddress() {

}

/**
 * @return string
 */
string InetAddress::getIp() {
    return string(inet_ntoa(_addr.sin_addr));
}

/**
 * @return unsigned short
 */
unsigned short InetAddress::getPort() {
    return ntohs(_addr.sin_port);
}

/**
 * @return const struct sockaddr_in *
 */
const struct sockaddr_in * InetAddress::getInetAddressPtr() {
    return &_addr;
}