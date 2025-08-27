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

    string line;
    char buf[4096];
    while (true)
    {
        int tag;
        cout << "input tag [1]:关键词推荐   [2]:网页搜索   [其他退出]: ";
        cin >> tag;
        if (tag != 1 && tag != 2) // ✅ 修正逻辑
            break;

        cin.ignore(); // 清掉缓冲区换行

        cout << "input query content: ";
        getline(cin, line);
        if (line == "exit") // ✅ 支持 exit 退出
            break;

        Message msg;
        msg.tag = tag;
        msg.length = line.size();
        msg.value = line;

        string packet = ProtocolParser::serialize(msg);
        send(sockfd, packet.c_str(), packet.size(), 0);

        // 接收数据
        memset(buf, 0, sizeof(buf));
        int n = recv(sockfd, buf, sizeof(buf) - 1, 0);
        if (n > 0)
        {
            cout << "[Server Reply]: " << endl;
            cout << buf << endl;
        }
        else if (n == 0)
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
