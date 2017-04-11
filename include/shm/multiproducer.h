#ifndef __MULTPRODUCER_HEADER__
#define __MULTIRODUCER_HEADER__

#include "shmmq_notify.h"

class MultiProducer
{
public:
    MultiProducer(const char *conf_path);
    ~MultiProducer();
    int sendData(const void *data, unsigned data_len);
private:
    int fd;//open flock file
    ShmBase::ShmMQNotify *shmmq_notifier;
};

#endif
