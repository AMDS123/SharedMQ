#include "multitproducer.hpp"

MutilTProducer:: MutilTProducer(const char *conf_path, pthread_mutex_t &_mutex): Producer(conf_path), mutex(_mutex)
{
}

int MutilTProducer::sendData(const void *data, unsigned data_len)
{
    pthread_mutex_lock(&mutex);
    int ret = Producer::sendData(data, data_len);
    pthread_mutex_unlock(&mutex);
    return ret;
}
