#include "shmmqcommu.hpp"

ShmMQCommu::ShmMQCommu(const char *conf_path)
{
    ConfigReader::getConfigReader().Load(conf_path);    
    const char *key_path = ConfigReader::getConfigReader().GetString("shm", "keypath", "").c_str();
    int id = ConfigReader::getConfigReader().GetNumber("shm", "id", 1);
    unsigned shmsize = ConfigReader::getConfigReader().GetNumber("shm", "shmsize", 10240);
    const char *fifo_path = ConfigReader::getConfigReader().GetString("fifo", "fifopath", "").c_str();

    smp = new ShmMqProcessor(key_path, id, shmsize, fifo_path);
    buffer_blob.capacity = 1024;
    buffer_blob.data = new char[buffer_blob.capacity];
}

ShmMQCommu::~ShmMQCommu()
{
    delete buffer_blob.data;
    delete smp;
}

int ShmMQCommu::sendData(const void *data, unsigned data_len)
{
    int ret = smp->produce(data, data_len);
    return ret;
}

int ShmMQCommu::listen(SHM_CALLBACK *call_back)
{   
    FOREVER
    {
        printf("read data...\n");
        int ret = smp->consume(buffer_blob.data, buffer_blob.capacity, buffer_blob.len);
        call_back->do_poll(&buffer_blob);
    }
    return 0;
}