#include "shmmqconsumer.hpp"
#include "errors.hpp"
#include "configreader.hpp"
#include <sys/epoll.h>

ShmMQConsumer::ShmMQConsumer(const char *conf_path)
{
    shmmq_operator = new ShmMQOperator(conf_path, READER);
    exit_if(shmmq_operator == NULL, "new ShmMQOperator");
    unsigned shmsize = ConfigReader::getConfigReader(conf_path)->GetNumber("shm", "shmsize", 10240);
    buffer_blob.capacity = shmsize;
    buffer_blob.data = new char[buffer_blob.capacity];
    exit_if(buffer_blob.data == NULL, "new blob");
}

ShmMQConsumer::~ShmMQConsumer()
{
    delete buffer_blob.data;
    delete shmmq_operator;
}

void ShmMQConsumer::readDataUntilEmpty(void)
{
    int ret;
    FOREVER
    {
        ret = shmmq_operator->begin_consume(buffer_blob.data, buffer_blob.capacity, buffer_blob.len);
        if (ret != 0)
        {
            break;
        }
        call_back->do_poll(&buffer_blob);
        shmmq_operator->finish_consume();
    }
}

int ShmMQConsumer::listen(SHM_CALLBACK *call_back)
{
    int poll_fd = epoll_create(10);
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = shmmq_operator->get_notify_fd();
    int ctl_ret = epoll_ctl(poll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    exit_if(ctl_ret < 0, "epoll_ctl");
    
    this->call_back = call_back;

    TELL_ERROR("begin to read shmmq.");
    readDataUntilEmpty();

    struct epoll_event events[10];
    FOREVER
    {
        int nfds = epoll_wait(poll_fd, events, 10, 5);
        if (nfds == -1)
        {
            TELL_ERROR("epoll_wait.");
            continue;
        }
        if (nfds > 0)
        {
            if (events[0].data.fd == shmmq_operator->get_notify_fd())
            {
                readDataUntilEmpty();
            }
            else
            {
                TELL_ERROR("impossible.");
            }
        }
    }
    return 0;
}
