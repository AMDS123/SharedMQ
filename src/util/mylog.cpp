#include "mylog.hpp"
#include "errors.hpp"
#include <stdio.h>

MyLog* MyLog::ins = NULL;

MyLog* MyLog::getInstance()
{
    exit_if(ins == NULL, "new MyLog");
    return ins;
}

MyLog::MyLog(const char* c_role): logger(log4cplus::Logger::getInstance(c_role))
{
}

void MyLog::init(const char *log_conf_path, Role role)
{
    if (!ins)
    {
        log4cplus::initialize();
        log4cplus::PropertyConfigurator::doConfigure(log_conf_path);
        if (role == READER)
        {
            ins = new MyLog("reader");
        }
        else if (role == WRITER)
        {
            ins = new MyLog("writer");
        }
    }
}
