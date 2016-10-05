#ifndef __SHMMQPROCESS_HEADER__
#define __SHMMQPROCESS_HEADER__

#include "shmmq.hpp"
#include "notify.hpp"

class ShmMQOperator
{
public:
    ShmMQOperator(const char* conf_path, Role role);
    ~ShmMQOperator();
    int get_notify_fd() const;
    int produce(const void* data, unsigned data_len);
    int consume(void* buffer, unsigned buffer_size, unsigned &data_len);

private:
    NotifyFileHandler* notify_fd_handler;
    ShmMQ* shmmq;
};

#endif