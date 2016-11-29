#ifndef __SINGLEPRODUCER_HEADER__
#define __SINGLEPRODUCER_HEADER__

#include "shmmqoperator.hpp"

class Producer
{
public:
    Producer(const char *conf_path);
    ~Producer();

    int sendData(const void *data, unsigned data_len);
private:
    ShmMQOperator *shmmq_operator;
};

#endif