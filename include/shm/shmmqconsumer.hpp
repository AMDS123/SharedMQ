#ifndef __SHMMQCONSUMER_HEADER__
#define __SHMMQCONSUMER_HEADER__

#include "shmmqoperator.hpp"

#define FOREVER while (1)

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

class ShmMQConsumer
{
public:
    ShmMQConsumer(const char *conf_path);
    ~ShmMQConsumer();

    int listen(SHM_CALLBACK *call_back);
private:
    void readDataUntilEmpty();

    Blob_Type buffer_blob;
    ShmMQOperator *shmmq_operator;
    SHM_CALLBACK *call_back;
};

#endif