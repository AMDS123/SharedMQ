#ifndef __MYLOG_HEADER__
#define __MYLOG_HEADER__

#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

#include "notify.hpp"

class MyLog
{
public:
    static MyLog *getInstance();
    static void init(const char *log_conf_path, Role role);
    log4cplus::Logger logger;
private:
    MyLog(const char* c_role);

    static MyLog* ins;
};

#define MYLOG_DEBUG(event) LOG4CPLUS_DEBUG(MyLog::getInstance()->logger, event)
#define MYLOG_INFO(event) LOG4CPLUS_INFO(MyLog::getInstance()->logger, event)
#define MYLOG_WARN(event) LOG4CPLUS_WARN(MyLog::getInstance()->logger, event)
#define MYLOG_ERROR(event) LOG4CPLUS_ERROR(MyLog::getInstance()->logger, event)
#define MYLOG_FATAL(event) LOG4CPLUS_FATAL(MyLog::getInstance()->logger, event)

#endif
