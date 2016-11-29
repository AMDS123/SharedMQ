#ifndef __MUTILPPRODUCER_HEADER__
#define __MUTILPPRODUCER_HEADER__

#include "singleproducer.hpp"

class MutilPProducer: public Producer
{
public:
    MutilPProducer(const char *conf_path);
    ~MutilPProducer();
    int sendData(const void *data, unsigned data_len);
private:
    int fd;//open flock file
};

#endif