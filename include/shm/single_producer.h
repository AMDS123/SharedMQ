#ifndef __SINGLEPRODUCER_HEADER__
#define __SINGLEPRODUCER_HEADER__

#include "shmmq_notify.h"

class Producer
{
public:
    Producer(const char *conf_path);
    ~Producer();

    int sendData(const void *data, unsigned data_len, std::string& err_msg);
private:
    ShmBase::ShmMQNotify *shmmq_notifier;
};

#endif