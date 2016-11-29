#include "multipproducer.hpp"
#include "scopedflock.hpp"
#include "errors.hpp"

#include <fcntl.h>

MutilPProducer::MutilPProducer(const char *conf_path): Producer(conf_path)
{
    fd = open("/tmp/producer.flock", O_CREAT | O_WRONLY);
    exit_if(fd == -1, "open producer flock");
}

MutilPProducer::~MutilPProducer()
{
    close(fd);
}

int MutilPProducer::sendData(const void *data, unsigned data_len)
{
    util::Scoped_Flock flock(fd);
    return Producer::sendData(data, data_len);
}
