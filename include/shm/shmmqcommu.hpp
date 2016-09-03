#ifndef __SHMMQCOMMU_HEADER__
#define __SHMMQCOMMU_HEADER__

#include "shmmqprocessor.hpp"
#include "configreader.hpp"

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

class ShmMQCommu
{
public:
    ShmMQCommu(const char *conf_path);
    ~ShmMQCommu();

    int sendData(const void *data, unsigned data_len);

    int listen(SHM_CALLBACK *call_back);

private:
    Blob_Type buffer_blob;
    ShmMqProcessor *smp;
};

#endif