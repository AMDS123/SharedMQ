#ifndef __SINGLECONSUMER_HEADER__
#define __SINGLECONSUMER_HEADER__

#include "shmmqoperator.hpp"
#include "errors.hpp"

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

    inline int readData(void* buffer, unsigned buffer_size, unsigned &data_len)
    {
        return read_one_data(buffer, buffer_size, data_len, -1);
    }

    inline int readDataNoWait(void* buffer, unsigned buffer_size, unsigned &data_len)
    {
        return read_one_data(buffer, buffer_size, data_len, 0);
    }

    inline int readDataTimeout(void* buffer, unsigned buffer_size, unsigned &data_len, int timeout/*milliseconds*/)
    {
        if (timeout > 0)
        {
            return read_one_data(buffer, buffer_size, data_len, timeout);
        }
        return ARG_ERR;
    }

private:
    int read_one_data(void* buffer, unsigned buffer_size, unsigned &data_len, int timeout);
    void readDataUntilEmpty(void);

    Blob_Type buffer_blob;
    ShmBase::ShmMQOperator *shmmq_operator;
    SHM_CALLBACK *call_back;
    int poll_fd;
};

#endif