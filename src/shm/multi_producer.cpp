#include <unistd.h>
#include <sys/file.h>
#include "errors.h"
#include "multi_producer.h"
#include "configreader.h"

MultiProducer::MultiProducer(const char *conf_path)
{
    const char *key_path = ConfigReader::getConfigReader(conf_path)->GetString("shm", "keypath", "").c_str();
    key_t key = ::ftok(key_path, 1);
    exit_if(key == (key_t)-1, "ftok key");

    char flock_name[256] = {};
    sprintf(flock_name, "/tmp/producer_%d.flock", key);

    fd = open(flock_name, O_CREAT | O_WRONLY);
    exit_if(fd == -1, "open producer flock");
    shmmq_notifier = new ShmBase::ShmMQNotify(conf_path, ShmBase::ShmMQNotify::WRITER);
    exit_if(shmmq_notifier == NULL, "new ShmMQNotify");
}

MultiProducer::~MultiProducer()
{
    close(fd);
    delete shmmq_notifier;
}

int MultiProducer::sendData(const void *data, unsigned data_len)
{
    std::string err_msg;
    int ret = flock(fd, LOCK_EX);
    if (0 == ret)
    {
        ret = shmmq_notifier->produce(data, data_len, err_msg);
        flock(fd, LOCK_UN);
    }
    return ret;
}
