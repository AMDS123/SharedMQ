#include <sys/epoll.h>
#include <errno.h>
#include "errors.h"
#include "configreader.h"
#include "single_consumer.h"

Consumer::Consumer(const char *conf_path)
{
    shmmq_notifier = new ShmBase::ShmMQNotify(conf_path, ShmBase::ShmMQNotify::READER);
    exit_if(shmmq_notifier == NULL, "new ShmMQNotify");
    unsigned msg_max_len = util::ConfigReader::getConfigReader(conf_path)->GetNumber("shm", "msg_max_len", 10240);
    buffer_blob.capacity = msg_max_len;
    buffer_blob.data = new char[msg_max_len];
    exit_if(buffer_blob.data == NULL, "new blob");

    //init epoll
    poll_fd = epoll_create1(EPOLL_CLOEXEC);
    exit_if(poll_fd == -1, "epoll_create");

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = shmmq_notifier->get_notify_fd();

    int ret = epoll_ctl(poll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    exit_if(ret < 0, "epoll_ctl");
}

Consumer::~Consumer()
{
    delete buffer_blob.data;
    delete shmmq_notifier;
}

void Consumer::loopReadData(void)
{
    std::string err_msg;
    int ret;
    while (true)
    {
        ret = shmmq_notifier->consume(buffer_blob.data, buffer_blob.capacity, buffer_blob.len, err_msg);
        if (ret != QUEUE_SUCC)
        {
            //TELL_ERROR(err_msg.c_str());
            break;
        }
        call_back->do_poll(&buffer_blob);
    }
}

int Consumer::listen(SHM_CALLBACK *call_back)
{
    this->call_back = call_back;
    //firstly, read data that already in shmmq
    loopReadData();
    struct epoll_event event;
    while (true)
    {
        //10ms wait
        int nfds = epoll_wait(poll_fd, &event, 1, 10);
        if (nfds > 0 && (event.events & EPOLLIN))
            loopReadData();
    }
    return 0;
}

int Consumer::read_one_data(void* buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg, int timeout)
{
    int ret = shmmq_notifier->consume(buffer, buffer_size, data_len, err_msg);

    //timeout = 0, noblock
    if (timeout == 0)
        return ret;

    //get data, and return ret
    if (ret != QUEUE_ERR_EMPTY)
        return ret;

    //no data in shmmq right now, so wait it
    struct epoll_event event;
    int nfds = epoll_wait(poll_fd, &event, 1, timeout);
    if (nfds == -1)
    {
        TELL_SYS_ERROR;
        return SYS_ERR;
    }
    if (nfds > 0 && (event.events & EPOLLIN))
    {
        return shmmq_notifier->consume(buffer, buffer_size, data_len, err_msg);
    }
    //no data yet, timeout
    return QUEUE_ERR_TIMEOUT;
}

