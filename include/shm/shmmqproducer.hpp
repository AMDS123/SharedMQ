#ifndef __SHMMQPRODUCER_HEADER__
#define __SHMMQPRODUCER_HEADER__

#include "shmmqoperator.hpp"

class ShmMQProducer
{
public:
    ShmMQProducer(const char *conf_path);
    ~ShmMQProducer();

    int sendData(const void *data, unsigned data_len);
private:
    ShmMQOperator *shmmq_operator;
};

#endif