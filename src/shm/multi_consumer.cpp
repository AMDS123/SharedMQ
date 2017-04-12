#include <unistd.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/epoll.h>
#include "configreader.h"
#include "multi_consumer.h"

MultiConsumer::MultiConsumer(const char *conf_path, int max_msg_cnt): buffer_blobs(max_msg_cnt)
{
    shmmq_notifier = new ShmBase::ShmMQNotify(conf_path, ShmBase::ShmMQNotify::READER);
    exit_if(shmmq_notifier == NULL, "new ShmMQNotify");

    unsigned msg_max_len = ConfigReader::getConfigReader(conf_path)->GetNumber("shm", "msg_max_len", 10240);
    for (int i = 0;i < max_msg_cnt; ++i)
    {
        buffer_blobs[i].capacity = msg_max_len;
        buffer_blobs[i].data = new char[msg_max_len];
        exit_if(buffer_blobs[i].data == NULL, "new blob");
    }

    //init epoll
    poll_fd = epoll_create1(EPOLL_CLOEXEC);
    exit_if(poll_fd == -1, "epoll_create");

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = shmmq_notifier->get_notify_fd();

    int ret = epoll_ctl(poll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    exit_if(ret < 0, "epoll_ctl");

    const char *key_path = ConfigReader::getConfigReader(conf_path)->GetString("shm", "keypath", "").c_str();
    key_t key = ::ftok(key_path, 1);
    exit_if(key == (key_t)-1, "ftok key");

    char flock_name[256] = {};
    sprintf(flock_name, "/tmp/consumer_%d.flock", key);
    fd = open(flock_name, O_CREAT | O_WRONLY);
    exit_if(fd == -1, "open producer flock");
}

MultiConsumer::~MultiConsumer()
{
    close(fd);
    for (int i = 0;i < buffer_blobs.size(); ++i)
        delete buffer_blobs[i].data;
    delete shmmq_notifier;
}

int MultiConsumer::listen(SHM_CALLBACK *call_back)
{
    this->call_back = call_back;
    //firstly, read data that already in shmmq. this is a very small probability event
    //loopReadData();
    struct epoll_event event;
    while (true)
    {
        //10ms wait
        int nfds = epoll_wait(poll_fd, &event, 1, 10);
        if (nfds > 0 && (event.events & EPOLLIN))
        {
            if (0 == flock(fd, LOCK_EX))
            {
                int blob_cnt = loopReadData();
                flock(fd, LOCK_UN);
                for (int i = 0;i < blob_cnt; ++i)
                    call_back->do_poll(&buffer_blobs[i]);
            }
        }
    }
    return 0;
}

int MultiConsumer::loopReadData(void)
{
    std::string err_msg;
    int ret;
    int i = 0;
    while (i < buffer_blobs.size())
    {
        ret = shmmq_notifier->consume(buffer_blobs[i].data, buffer_blobs[i].capacity, buffer_blobs[i].len, err_msg);
        if (ret != QUEUE_SUCC)
            break;
        ++i;
    }
    return i;
}

int MultiConsumer::read_one_data(void* buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg, bool block)
{
    int ret = block? flock(fd, LOCK_EX) : flock(fd, LOCK_EX | LOCK_NB);
    if (ret == 0)
    {
        ret = shmmq_notifier->consume(buffer, buffer_size, data_len, err_msg);
        flock(fd, LOCK_UN);
    }
    return ret;
}
