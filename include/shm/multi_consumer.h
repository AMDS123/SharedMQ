#ifndef __MULTICONSUMER_HEADER__
#define __MULTICONSUMER_HEADER__

#include <vector>
#include "shmmq_notify.h"
#include "single_consumer.h"

class MultiConsumer
{
public:
    MultiConsumer(const char *conf_path, int max_msg_cnt = 10);
    ~MultiConsumer();
    int listen(SHM_CALLBACK *call_back);
    int read_one_data(void* buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg, bool block = true);

private:
    int loopReadData(void);

    int fd;//open flock file
    ShmBase::ShmMQNotify *shmmq_notifier;
    SHM_CALLBACK *call_back;
    int poll_fd;

    std::vector<Blob_Type> buffer_blobs;
};

#endif