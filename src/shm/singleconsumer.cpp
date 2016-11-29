#include "singleconsumer.hpp"
#include "errors.hpp"
#include "configreader.hpp"
#include <sys/epoll.h>
#include <errno.h>

Consumer::Consumer(const char *conf_path)
{
    shmmq_operator = new ShmMQOperator(conf_path, READER);
    exit_if(shmmq_operator == NULL, "new ShmMQOperator");
    unsigned shmsize = ConfigReader::getConfigReader(conf_path)->GetNumber("shm", "shmsize", 10240);
    buffer_blob.capacity = shmsize;
    buffer_blob.data = new char[buffer_blob.capacity];//ERROR, should add new configure about one message MAXSIZE
    exit_if(buffer_blob.data == NULL, "new blob");

    //init epoll
    poll_fd = epoll_create(1);
    exit_if(poll_fd == -1, "epoll_create");
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = shmmq_operator->get_notify_fd();
    int ctl_ret = epoll_ctl(poll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    exit_if(ctl_ret < 0, "epoll_ctl");
}

Consumer::~Consumer()
{
    delete buffer_blob.data;
    delete shmmq_operator;
}

void Consumer::readDataUntilEmpty(void)
{
    int ret;
    while (true)
    {
        ret = shmmq_operator->begin_consume(buffer_blob.data, buffer_blob.capacity, buffer_blob.len);
        if (ret != 0)
        {
            break;
        }
        shmmq_operator->finish_consume();
        call_back->do_poll(&buffer_blob);
    }
}

int Consumer::read_one_data(void* buffer, unsigned buffer_size, unsigned &data_len, int timeout)
{
    struct epoll_event events[1];
    int nfds = epoll_wait(poll_fd, events, 1, timeout);
    if (nfds == -1)
    {
        TELL_SYS_ERROR;
    }
    else if (nfds > 0)
    {
        if (events[0].data.fd == shmmq_operator->get_notify_fd())
        {
            int ret = shmmq_operator->begin_consume(buffer, buffer_size, data_len);
            shmmq_operator->finish_consume();
            return ret;
        }
        else
        {
            TELL_ERROR("impossible.");
        }
    }
    return nfds == 0 ? NODATA_ERR: SYS_ERR;
}

int Consumer::listen(SHM_CALLBACK *call_back)
{   
    this->call_back = call_back;

    TELL_ERROR("begin to read shmmq.");
    readDataUntilEmpty();

    struct epoll_event events[1];
    while (true)
    {
        int nfds = epoll_wait(poll_fd, events, 1, 5);
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
