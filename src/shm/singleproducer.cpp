#include "singleproducer.hpp"
#include "errors.hpp"
#include "configreader.hpp"

Producer::Producer(const char *conf_path)
{
    shmmq_operator = new ShmBase::ShmMQOperator(conf_path, util::WRITER);
    exit_if(shmmq_operator == NULL, "new ShmMQOperator");
}

Producer::~Producer()
{
    delete shmmq_operator;
}

int Producer::sendData(const void *data, unsigned data_len)
{
    int ret = shmmq_operator->produce(data, data_len);
    return ret;
}