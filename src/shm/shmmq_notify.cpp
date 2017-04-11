#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "errors.h"
#include "shmmq_notify.h"
#include "configreader.h"

namespace ShmBase
{

ShmMQNotify::ShmMQNotify(const char* conf_path, Role role): read_fifo_cnt(0), write_fifo_cnt(0)
{
    this->role = role;
    shmmq = new ShmMQ(conf_path);
    exit_if(shmmq == NULL, "new ShmMQ");

    const char *fifo_path = ConfigReader::getConfigReader(conf_path)->GetString("fifo", "fifopath", "leechanx_fifo").c_str();
    if (::access(fifo_path, F_OK) == -1)
    {
        int ret = ::mkfifo(fifo_path, 0666);
        exit_if((ret == -1 && errno != EEXIST), "mkfifo");//atomic create fifo
    }

    if (role == READER)
        //reader发现writer已经关闭，FIFO一直可读EOF，使得在没有新写者开始写之前，reader总是有无意义的可读事件给epoll，吃爆CPU
        //把reader改为读写打开即可，使得保证至少有一个writer永远存在，只不过这个writer不写数据，只充当读者
        notify_fd = ::open(fifo_path, O_RDWR, 0666);//TIPS [1]
    else if (role == WRITER)
    {
        //when reader processor terminated, don't let SIGPIPE terminate writer processor
        signal(SIGPIPE,SIG_IGN);
        notify_fd = ::open(fifo_path, O_WRONLY, 0666);
    }

    exit_if(notify_fd == -1, "open fifo");

    int flag = ::fcntl(notify_fd, F_GETFL, 0);
    ::fcntl(notify_fd, F_SETFL, O_NONBLOCK | flag);
}

ShmMQNotify::~ShmMQNotify()
{
    delete shmmq;
    ::close(notify_fd);
}

int ShmMQNotify::produce(const void *data, unsigned data_len, std::string& err_msg)
{
    if (role != WRITER)
        return NOAUTHOR_CALL;
    int ret = shmmq->enqueue(data, data_len, err_msg);
    if (ret == QUEUE_SUCC)
        //notify
        ::write(notify_fd, "!", 1);
    return ret;
}

int ShmMQNotify::consume(void *buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg)
{
    if (role != READER)
        return NOAUTHOR_CALL;
    int ret = shmmq->dequeue(buffer, buffer_size, data_len, err_msg);
    char fbuf;
    ::read(notify_fd, &fbuf, 1);
    return ret;
}

}
