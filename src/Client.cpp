#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "ProtocolParser.hpp"
using namespace std;

int main(int argc, char *argv[])
{
    string ip = "127.0.0.1";
    int port = 8888;

    if (argc == 3)
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    if (connect(sockfd, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    cout << "Connected to " << ip << ":" << port << endl;
    cout << "请输入查询词(输入 exit 退出):" << endl;

    string line;
    char buf[4096];
    while (true)
    {
        cout << "> ";
        getline(cin, line);
        if (line == "exit")
            break;

        Message msg;
        msg.tag = 1;
        msg.length = line.size();
        msg.value = line;
        string packet = ProtocolParser::serialize(msg);
        send(sockfd, packet.c_str(), packet.size(), 0);

        // 接收数据
        memset(buf, 0, sizeof(buf));
        int n = recv(sockfd, buf, sizeof(buf) - 1, 0);
        if (n > 0)
        {
            cout << "[Server Reply]: "  << endl;
            cout << buf << endl;

        }
        else if (n == 0)// 对端断开
        {
            cout << "Server closed connection." << endl;
            break;
        }
        else
        {
            perror("recv");
            break;
        }
    }

    close(sockfd);
    return 0;
}
