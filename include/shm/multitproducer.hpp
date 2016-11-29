#ifndef __MUTILTPRODUCER_HEADER__
#define __MUTILTPRODUCER_HEADER__

#include "singleproducer.hpp"

#include <pthread.h>

class MutilTProducer: public Producer
{
public:
    MutilTProducer(const char *conf_path, pthread_mutex_t &_mutex);
    int sendData(const void *data, unsigned data_len);
private:
    pthread_mutex_t &mutex;
};

#endif