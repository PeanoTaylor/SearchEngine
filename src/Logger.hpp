#ifndef LOGGER_H
#define LOGGER_H
#include <iostream>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Category.hh>
using namespace std;
using namespace log4cpp;

#define LOG_WARN(msg) Logger::getInstance()->warn(__FILE__,__LINE__,__FUNCTION__,msg)
#define LOG_ERROR(msg) Logger::getInstance()->error(__FILE__,__LINE__,__FUNCTION__,msg)
#define LOG_DEBUG(msg) Logger::getInstance()->debug(__FILE__,__LINE__,__FUNCTION__,msg)
#define LOG_INFO(msg) Logger::getInstance()->info(__FILE__,__LINE__,__FUNCTION__,msg)




class Logger{
public:
    void warn(const char * file, int line, const char * func, const char * msg);
    void error(const char * file, int line, const char * func, const char * msg);
    void debug(const char * file, int line, const char * func, const char * msg);
    void info(const char * file, int line, const char * func, const char * msg);

    static Logger * getInstance();

    static void destroy();


private:
    Logger();
    ~Logger();
    Logger(const Logger &) = delete;
    Logger & operator=(const Logger &) = delete;
    static Logger * _pInstance;
    static Category * _root;
};


#endif