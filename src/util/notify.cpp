#include "notify.hpp"
#include "errors.hpp"
#include "configreader.hpp"

void HANDLE_SIGPIPE(int signo)
{
	TELL_ERROR("receive SIGPIPE signal!");
}

FifoFd::FifoFd(const char *conf_path, Role role)
{
    const char *fifo_path = ConfigReader::getConfigReader(conf_path)->GetString("fifo", "fifopath", "leechanx_fifo").c_str();
    if (access(fifo_path, F_OK) == -1)
    {
        int ret = mkfifo(fifo_path, 0666);
        exit_if(ret < 0, "mkfifo");
    }

    if (role == READER)
    {
        notify_fd = open(fifo_path, O_RDONLY, 0666);
    }
    else if (role == WRITER)
    {
    	notify_fd = open(fifo_path, O_WRONLY, 0666);
        signal(SIGPIPE, HANDLE_SIGPIPE);
    }    
    exit_if(notify_fd == -1, "open fifo");
}

int FifoFd::notify_event()
{
	int wn;
    if((wn = write(notify_fd, "!", 1)) < 0)
    {
        TELL_ERROR("write fifo");
    }
    return wn;
}

int FifoFd::receive_event()
{
    int rn;
    char temp_buffer;
    if ((rn = read(notify_fd, &temp_buffer, 1)) < 0)
    {
        TELL_ERROR("read notify");
    }
    return rn;
}

EventFd::EventFd(const char *conf_path, Role role)
{
    const char *sun_path = ConfigReader::getConfigReader(conf_path)->GetString("eventfd", "unixsocketpath", "leechanx_unix_path").c_str();
    int socketfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    exit_if(socketfd == -1, "socket");
    
    struct sockaddr_un servaddr;
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, sun_path);
    int ret;

    if (role == WRITER)
    {   
        unlink(sun_path);

        ret = bind(socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        exit_if(ret == -1, "bind");
    
        ret = listen(socketfd, 5);
        exit_if(ret == -1, "listen");

        struct sockaddr_un client_addr;
        socklen_t client_len;
    
        int connfd = accept(socketfd, (struct sockaddr *)&client_addr, &client_len);
        exit_if(connfd == -1, "accept");

        char c;
        ret = recv_fd(connfd, &c, 1);
        exit_if(ret == -1, "recv_fd");

        exit_if(notify_fd == -1, "get eventfd handler");
    }
    else if (role == READER)
    {
        ret = connect(socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        while (ret == -1)
        {
            sleep(1);
            ret = connect(socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        }

        notify_fd = eventfd(0, EFD_NONBLOCK);//[ref] http://linux.die.net/man/2/eventfd
        exit_if(notify_fd == -1, "eventfd(0, 0)");

        char c = '!';
        ret = send_fd(socketfd, &c, 1);
        exit_if(ret == -1, "send_fd");
    }
}

int EventFd::notify_event()
{
    unsigned long long number = 1;
    int ret;
    if ((ret = write(notify_fd, &number, sizeof(unsigned long long))) < 0)
    {
        TELL_ERROR("write eventfd");
    }
    return ret;
}

int EventFd::receive_event()
{
    unsigned long long number;
    int ret;
    if ((ret = read(notify_fd, &number, sizeof(unsigned long long))) < 0)
    {
        TELL_ERROR("read eventfd");
    }
    return ret;
}

ssize_t EventFd::send_fd(int sockfd, void* data, size_t bytes)
{
    struct msghdr msghdr_send;
    struct iovec iov[1];
    size_t  n;
    int newfd;
    union
    {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    struct cmsghdr *pcmsghdr = NULL;
    msghdr_send.msg_control = control_un.control;
    msghdr_send.msg_controllen = sizeof(control_un.control);

    pcmsghdr = CMSG_FIRSTHDR(&msghdr_send);
    pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));
    pcmsghdr->cmsg_level = SOL_SOCKET;
    pcmsghdr->cmsg_type = SCM_RIGHTS;
    *(int*)CMSG_DATA(pcmsghdr) = notify_fd;

    msghdr_send.msg_name = NULL;
    msghdr_send.msg_namelen = 0;

    iov[0].iov_base = data;
    iov[0].iov_len  = bytes;
    msghdr_send.msg_iov = iov;
    msghdr_send.msg_iovlen = 1;

    return sendmsg(sockfd, &msghdr_send, 0);
}

ssize_t EventFd::recv_fd(int sockfd, void *data, size_t types)
{
    struct msghdr msghdr_recv;
    struct iovec iov[1];
    size_t n;
    int newfd;

    union
    {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    struct cmsghdr *pcmsghdr;
    msghdr_recv.msg_control = control_un.control;
    msghdr_recv.msg_controllen = sizeof(control_un.control);

    msghdr_recv.msg_name = NULL;
    msghdr_recv.msg_name = 0;

    iov[0].iov_base = data;
    iov[0].iov_len = types;
    msghdr_recv.msg_iov =iov;
    msghdr_recv.msg_iovlen =1;

    if ((n=recvmsg(sockfd, &msghdr_recv, 0)) <= 0)
    {
        return n;
    }

    if ((pcmsghdr = CMSG_FIRSTHDR(&msghdr_recv))!=NULL && pcmsghdr->cmsg_len == CMSG_LEN(sizeof(int)))
    {
        if (pcmsghdr->cmsg_level != SOL_SOCKET)
        {
            printf("control level != SCM_RIGHTS\n");
        }

        if (pcmsghdr->cmsg_type != SCM_RIGHTS)
        {
            printf("control type != SCM_RIGHTS");
        }    

        notify_fd = *(int*)CMSG_DATA(pcmsghdr);
        //printf("redvfd = %d\n",*recvfd);
    }
    else
    {
        notify_fd = -1;
    }
    return n;
}
