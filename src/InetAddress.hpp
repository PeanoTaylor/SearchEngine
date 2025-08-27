/**
 * Project 66th
 */


#pragma once
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/epoll.h>
using std::cout;
using std::endl;
using std::string;
class InetAddress {
public: 
    
/**
 * @param ip
 * @param port
 */
InetAddress(const string & ip, unsigned short port);
    
/**
 * @param addr
 */
InetAddress(const struct sockaddr_in & addr);
    
~InetAddress();
    
string getIp();
    
unsigned short getPort();
    
const struct sockaddr_in * getInetAddressPtr();
private: 
    struct sockaddr_in _addr;
};
