#include "shmmqcommu.hpp"
#include <fstream>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

long int getCurrentTimeInMillis()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}

char buff[2600];

int main()
{
    ShmMQCommu smp("../../conf/test.ini");
    sleep(10);
    printf("start!\n");
    std::ofstream ofs;
    ofs.open ("sender.txt", std::ofstream::out);

    srand(time(NULL));

    for (int i = 0;i < 2600; ++i)
    {
        buff[i] = i % 26 + 'a';
    }

    #define SCALE 100000
    bool flag = false;

    for (int i = 0;i < SCALE; ++i)
    {
        unsigned len = rand() % 10 + 16;
        if (smp.sendData(buff, len) == 0)
        {
            ofs << std::string(buff, len) << "+" << getCurrentTimeInMillis() << std::endl;
        }
        else
        {
            if (flag == false)
            {
                flag = true;
                ofs << std::string(buff, len) << "+" << i << " wocao" << std::endl;
            }
        }
    }
    ofs.close();
}
