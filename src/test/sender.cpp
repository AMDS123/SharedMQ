#include "shmmqcommu.hpp"

int main()
{
    ShmMQCommu smp("../conf/test.ini");
    
    sleep(10);
    const char *fuck1 = "leechanx is niubi";
    smp.sendData(fuck1, strlen(fuck1));
    
    const char *fuck2 = "liujiani is meili";
    smp.sendData(fuck2, strlen(fuck2));
    
    const char *fuck3 = "guojiexiao is keai";
    smp.sendData(fuck3, strlen(fuck3));
    sleep(1);

    const char *fuck4 = "zhangshuang is meibaole";
    smp.sendData(fuck4, strlen(fuck4));
}
