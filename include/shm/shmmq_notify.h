#ifndef __SHMMQ_NOTIFY_HEADER__
#define __SHMMQ_NOTIFY_HEADER__

#include "shmmq.h"

namespace ShmBase
{

class ShmMQNotify
{
public:
    enum Role
    {
        READER = 15,
        WRITER
    };

    ShmMQNotify(const char* conf_path, Role role);
    ~ShmMQNotify();

    int produce(const void* data, unsigned data_len, std::string& err_msg);
    int consume(void* buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg);
    int get_notify_fd() const { return notify_fd; }

private:
    int notify_fd;
    Role role;
    ShmMQ* shmmq;

    int read_fifo_cnt;//DEBUG
    int write_fifo_cnt;//DEBUG
};

}

#endif
