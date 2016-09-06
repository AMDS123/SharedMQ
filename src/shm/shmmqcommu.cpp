#include "shmmqcommu.hpp"
//#include <sys/epoll.h>

ShmMQCommu::ShmMQCommu(const char *conf_path)
{
    ConfigReader::getConfigReader().Load(conf_path);    
    const char *key_path = ConfigReader::getConfigReader().GetString("shm", "keypath", "").c_str();
    int id = ConfigReader::getConfigReader().GetNumber("shm", "id", 1);
    unsigned shmsize = ConfigReader::getConfigReader().GetNumber("shm", "shmsize", 10240);
    const char *fifo_path = ConfigReader::getConfigReader().GetString("fifo", "fifopath", "").c_str();

    smp = new ShmMqProcessor(key_path, id, shmsize, fifo_path);
    buffer_blob.capacity = shmsize;
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
    /*
    int poll_fd = epoll_create(1000);
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = smp->get_notify_fd();
    int ctl_ret = epoll_ctl(poll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    if (ctl_ret < 0)
    {
        perror("fuck you");
        exit(1);
    }

    struct epoll_event events[1000];
    */
    int ret;

    FOREVER
    {
        ret = smp->consume(buffer_blob.data, buffer_blob.capacity, buffer_blob.len, true);
        if (ret != 0)
        {
            break;
        }
        call_back->do_poll(&buffer_blob);
    }

    FOREVER
    {
        //int nfds = epoll_wait(poll_fd, events, 1000, 5);
        
        //int ret = smp->consume(buffer_blob.data, buffer_blob.capacity, buffer_blob.len);
        //call_back->do_poll(&buffer_blob);
        if (smp->consume(buffer_blob.data, buffer_blob.capacity, buffer_blob.len) == 0)
        {
            call_back->do_poll(&buffer_blob);
        }
    }
    return 0;
}
