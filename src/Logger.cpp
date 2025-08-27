#include "Logger.hpp"
#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>

// 初始化静态成员变量
// 初始化静态成员变量
Logger *Logger::_pInstance = nullptr;
Category *Logger::_root = nullptr;

// 构造函数
Logger::Logger()
{
    log4cpp::PatternLayout *layout1 = new log4cpp::PatternLayout();
    layout1->setConversionPattern("[%d] [%p] %c: %m%n");

    log4cpp::PatternLayout *layout2 = new log4cpp::PatternLayout();
    layout2->setConversionPattern("[%d] [%p] %c: %m%n");

    log4cpp::PatternLayout *layout3 = new log4cpp::PatternLayout();
    layout3->setConversionPattern("[%d] [%p] %c: %m%n");

    // 终端输出
    OstreamAppender *consoleAppender = new OstreamAppender("console", &std::cout);
    consoleAppender->setLayout(layout1);
    // 文件输出
    FileAppender *fileAppender = new FileAppender("file", "../log/log.txt");
    fileAppender->setLayout(layout2);

    // 回卷文件输出（自动滚动）
    log4cpp::RollingFileAppender *rollingAppender = new log4cpp::RollingFileAppender(
        "rolling",         // 名称
        "../log/rolling_log.txt", // 文件名
        5 * 1024 * 1024,   // 最大文件大小（5MB）
        3                  // 最大备份数量
    );
    rollingAppender->setLayout(layout3);

    // 设置输出信息
    _root = &Category::getRoot();
    _root->addAppender(consoleAppender);
    _root->addAppender(fileAppender);
    _root->addAppender(rollingAppender);
    _root->setPriority(Priority::DEBUG);
}

// 析构函数
Logger::~Logger()
{
    if (_root)
    {
        _root->removeAllAppenders();
    }
    Category::shutdown(); // 清理资源
}

// 单例方法
Logger *Logger::getInstance()
{
    if (!_pInstance)
    {
        _pInstance = new Logger();
    }
    return _pInstance;
}

// 单例销毁方法
void Logger::destroy()
{
    if (_pInstance)
    {
        delete _pInstance;
        _pInstance = nullptr;
    }
}

// 日志输出方法
void Logger::warn(const char *file, int line, const char *func, const char *msg)
{
    _root->warn("[%s:%d:%s] %s", file, line, func, msg);
}

void Logger::error(const char *file, int line, const char *func, const char *msg)
{
    _root->error("[%s:%d:%s] %s", file, line, func, msg);
}

void Logger::debug(const char *file, int line, const char *func, const char *msg)
{
    _root->debug("[%s:%d:%s] %s", file, line, func, msg);
}

void Logger::info(const char *file, int line, const char *func, const char *msg)
{
    _root->info("[%s:%d:%s] %s", file, line, func, msg);
}
