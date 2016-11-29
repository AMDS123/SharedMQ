#include "multipconsumer.hpp"
#include "scopedflock.hpp"
#include "errors.hpp"

MutilPConsumer::MutilPConsumer(const char *conf_path): Consumer(conf_path)
{
    fd = open("/tmp/producer.flock", O_CREAT | O_WRONLY);
    exit_if(fd == -1, "open producer flock");
}

MutilPConsumer::~MutilPConsumer()
{
    close(fd);
}

int MutilPConsumer::readData(void* buffer, unsigned buffer_size, unsigned &data_len)
{
    util::Scoped_Flock flock(fd);
    return Consumer::readData(buffer, buffer_size, data_len);
}

int MutilPConsumer::readDataNoWait(void* buffer, unsigned buffer_size, unsigned &data_len)
{
    util::Scoped_Flock flock(fd);
    return Consumer::readDataNoWait(buffer, buffer_size, data_len);
}

int MutilPConsumer::readDataTimeout(void* buffer, unsigned buffer_size, unsigned &data_len, int timeout/*milliseconds*/)
{
    util::Scoped_Flock flock(fd);
    return Consumer::readDataTimeout(buffer, buffer_size, data_len, timeout);
}