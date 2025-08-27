#include "SearchEngineServer.hpp"
#include <iostream>
#include <csignal>

using namespace std;

// 全局 server 指针，用于信号处理
SearchEngineServer *g_server = nullptr;

void signalHandler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        cout << "\n[Main] Caught signal " << sig << ", stopping server..." << endl;
        if (g_server) {
            g_server->stop();
        }
        exit(0);
    }
}

int main(int argc, char **argv)
{
    string ip = "127.0.0.1";
    int port = 8888;

    if (argc == 3) {
        ip = argv[1];
        port = atoi(argv[2]);
    }

    cout << "[Main] Starting SearchEngineServer on " << ip << ":" << port << endl;

    SearchEngineServer server(ip, port);
    g_server = &server;

    // 注册信号处理函数，方便 Ctrl+C 停止服务
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    server.start();

    return 0;
}
