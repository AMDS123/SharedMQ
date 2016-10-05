#include "shmmqproducer.hpp"
#include "errors.hpp"
#include "configreader.hpp"

ShmMQProducer::ShmMQProducer(const char *conf_path)
{
    shmmq_operator = new ShmMQOperator(conf_path, WRITER);
    exit_if(shmmq_operator == NULL, "new ShmMQOperator");
}

ShmMQProducer::~ShmMQProducer()
{
    delete shmmq_operator;
}

int ShmMQProducer::sendData(const void *data, unsigned data_len)
{
    int ret = shmmq_operator->produce(data, data_len);
    return ret;
}