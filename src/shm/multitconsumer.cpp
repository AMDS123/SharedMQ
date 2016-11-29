#include "multitconsumer.hpp"

MutilTConsumer::MutilTConsumer(const char *conf_path, pthread_mutex_t &_mutex): Consumer(conf_path), mutex(_mutex)
{
}

int MutilTConsumer::readData(void* buffer, unsigned buffer_size, unsigned &data_len)
{
    pthread_mutex_lock(&mutex);
    int ret = Consumer::readData(buffer, buffer_size, data_len);
    pthread_mutex_unlock(&mutex);
    return ret;
}

int MutilTConsumer::readDataNoWait(void* buffer, unsigned buffer_size, unsigned &data_len)
{
    pthread_mutex_lock(&mutex);
    int ret = Consumer::readDataNoWait(buffer, buffer_size, data_len);
    pthread_mutex_unlock(&mutex);
    return ret;
}

int MutilTConsumer::readDataTimeout(void* buffer, unsigned buffer_size, unsigned &data_len, int timeout/*milliseconds*/)
{
    pthread_mutex_lock(&mutex);
    int ret = Consumer::readDataTimeout(buffer, buffer_size, data_len, timeout);
    pthread_mutex_unlock(&mutex);
    return ret;
}
