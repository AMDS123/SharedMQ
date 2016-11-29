#include "singleproducer.hpp"
#include <fstream>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

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
    Producer shmwriter("../../conf/test.ini");
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
        if (shmwriter.sendData(buff, len) == 0)
        {
            std::cout << "send data" << std::endl;
            ofs << std::string(buff, len) << "+" << getCurrentTimeInMillis() << std::endl;
        }
    }
    ofs.close();
}
