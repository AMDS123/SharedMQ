#include "errors.h"
#include "configreader.h"
#include "single_producer.h"

Producer::Producer(const char *conf_path)
{
    shmmq_notifier = new ShmBase::ShmMQNotify(conf_path, ShmBase::ShmMQNotify::WRITER);
    exit_if(shmmq_notifier == NULL, "new ShmMQNotify");
}

Producer::~Producer()
{
    delete shmmq_notifier;
}

int Producer::sendData(const void *data, unsigned data_len, std::string& err_msg)
{
    return shmmq_notifier->produce(data, data_len, err_msg);
}
