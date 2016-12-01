#ifndef __NOTIFY_HEADER__
#define __NOTIFY_HEADER__

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/eventfd.h>
#include <linux/un.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

namespace util
{

enum Role
{
    READER = 15,
    WRITER
};

class NotifyFileHandler
{
public:
    NotifyFileHandler(): notify_fd(-1)
    {
    }
    virtual ~NotifyFileHandler()
    {
        close(notify_fd);
    }
    int get_notify_fd() const
    {
        return notify_fd;
    }
    virtual int notify_event() = 0;
    virtual int receive_event() = 0;
protected:
    int notify_fd;
};

class FifoFd: public NotifyFileHandler
{
public:
    FifoFd(const char *conf_path, Role role);
    virtual int notify_event();
    virtual int receive_event();
};

class EventFd: public NotifyFileHandler
{
public:
    EventFd(const char *conf_path, Role role);
    virtual int notify_event();
    virtual int receive_event();
private:
    ssize_t send_fd(int sockfd, void* data, size_t bytes);
    ssize_t recv_fd(int sockfd, void *data, size_t types);
};

}

#endif