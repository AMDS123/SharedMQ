#ifndef __SHMMQPROCESS_HEADER__
#define __SHMMQPROCESS_HEADER__

#include "shmmq.hpp"
#include "notify.hpp"

class ShmMqProcessor
{
public:
    ShmMqProcessor(const char* conf_path, Role role);
    ~ShmMqProcessor();
    int get_notify_fd() const;
    int produce(const void* data, unsigned data_len);
    int consume(void* buffer, unsigned buffer_size, unsigned &data_len);

private:
    NotifyFileHandler* notify_fd_handler;
    ShmMQ* shmmq;
};

#endif