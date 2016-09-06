#include "shmmqprocessor.hpp"
#include "errors.hpp"

ShmMqProcessor::ShmMqProcessor(const char *key_path, int id, unsigned shmsize, const char *fifo_path)
{
    shmmq = new ShmMQ(key_path, id, shmsize);
    if (access(fifo_path, F_OK) == -1)
    {
        int ret = mkfifo(fifo_path, 0666);
        exit_if(ret < 0, "mkfifo");
    }
    notify_fd = open(fifo_path, O_RDWR, 0666);
    exit_if(notify_fd == -1, "open fifo");
}

ShmMqProcessor::~ShmMqProcessor()
{
    delete shmmq;
    close(notify_fd);
}

void ShmMqProcessor::debug()
{
    shmmq->debug();
}

void ShmMqProcessor::notify()
{
    if(write(notify_fd, "!", 1) < 0)
    {
        TELL_ERROR("notify error.");
    }
}

int ShmMqProcessor::produce(const void *data, unsigned data_len)
{
    int ret;
    ret = shmmq->enqueue(data, data_len);
    notify();
    return ret;
}

int ShmMqProcessor::consume(void *buffer, unsigned buffer_size, unsigned &data_len)
{
    char temp_buffer;
    if (read(notify_fd, &temp_buffer, 1) < 0)
    {
        TELL_ERROR("read notify error.");
    }
    int ret;
    ret = shmmq->dequeue(buffer, buffer_size, data_len);
    return ret;
}

int ShmMqProcessor::get_notify_fd()
{
    return notify_fd;
}