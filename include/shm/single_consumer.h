#ifndef __SINGLECONSUMER_HEADER__
#define __SINGLECONSUMER_HEADER__

#include "shmmq_notify.h"
#include "errors.h"

typedef struct Blob
{
    char *data;
    unsigned len;
    unsigned capacity;
} Blob_Type;

class SHM_CALLBACK
{
public:
    virtual void do_poll(Blob_Type *buffer_blob) = 0;
};

class Consumer
{
public:
    Consumer(const char *conf_path);
    ~Consumer();

    int listen(SHM_CALLBACK *call_back);

    inline int readData(void* buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg, int timeout = -1)
    {
        if (timeout < -1)
            timeout = -1;
        return read_one_data(buffer, buffer_size, data_len, err_msg, timeout);
    }

    inline int readDataNoWait(void* buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg)
    {
        return read_one_data(buffer, buffer_size, data_len, err_msg, 0);
    }

private:
    /*
      read_one_data: read a data from shmmq
      buffer: read buffer
      buffer_size: read buffer size
      data_len: data length
      timeout (ms): max waiting time before data arrive in shmmq
        timeout = 0: no block
        timeout = -1: always block
        timeout > 0: wait millis
    */
    int read_one_data(void* buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg, int timeout);

    void loopReadData(void);

    Blob_Type buffer_blob;
    ShmBase::ShmMQNotify *shmmq_notifier;
    SHM_CALLBACK *call_back;
    int poll_fd;
};

#endif
