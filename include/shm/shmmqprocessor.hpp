#ifndef __SHMMQPROCESS_HEADER__
#define __SHMMQPROCESS_HEADER__

#include "shmmq.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

class ShmMqProcessor
{
public:
    ShmMqProcessor(const char *key_path, int id, unsigned shmsize, const char *fifo_path);
    
    ~ShmMqProcessor();

    void notify();

    void debug();

    int produce(const void *data, unsigned data_len);

    int consume(void *buffer, unsigned buffer_size, unsigned &data_len, bool firstTime = false);

    int get_notify_fd();

private:
    int notify_fd;
    ShmMQ *shmmq;
};

#endif