#ifndef __MULTITCONSUMER_HEADER__
#define __MULTITCONSUMER_HEADER__

#include "singleconsumer.hpp"

#include <pthread.h>

class MutilTConsumer: public Consumer
{
public:
    MutilTConsumer(const char *conf_path, pthread_mutex_t &_mutex);
    int readData(void* buffer, unsigned buffer_size, unsigned &data_len);
    int readDataNoWait(void* buffer, unsigned buffer_size, unsigned &data_len);
    int readDataTimeout(void* buffer, unsigned buffer_size, unsigned &data_len, int timeout/*milliseconds*/);
private:
    pthread_mutex_t &mutex;
};

#endif