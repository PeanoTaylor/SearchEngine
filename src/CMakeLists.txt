cmake_minimum_required(VERSION 3.15)

# 项目名称
project(SearchEngine)

# 指定C++的版本
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)

# 生成的可执行程序
add_executable(SearchEngine
    Acceptor.cpp
    EventLoop.cpp
    InetAddress.cpp
    KeyRecommander.cpp
    WebPageSearcher.cpp
    Logger.cpp
    main.cpp
    ProtocolParser.cpp
    SearchEngineServer.cpp
    Socket.cpp
    SocketIO.cpp
    TaskQueue.cpp
    TcpConnection.cpp
    TcpServer.cpp
    ThreadPool.cpp
)

target_compile_options(SearchEngine PRIVATE
    -g
)

target_link_libraries(SearchEngine PRIVATE
    pthread
    log4cpp
    tinyxml2
)

add_executable(Client
    Client.cpp
    ProtocolParser.cpp
)

target_compile_options(Client PRIVATE
    -g
)

target_link_libraries(Client PRIVATE
    pthread
    log4cpp
    tinyxml2
)

add_executable(Corpus
    buildCorpus.cpp
    KeywordProcessor.cpp
    PageProcessor.cpp
    DirectoryScanner.cpp
)

target_compile_options(Corpus PRIVATE
    -g
)

target_link_libraries(Corpus PRIVATE
    log4cpp
    tinyxml2
)