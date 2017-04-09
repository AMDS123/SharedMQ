#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include "single_producer.h"
#include <list>

using namespace std;

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

    #if 0
    #define SCALE 100000
    std::string err_msg;

    for (int i = 0;i < SCALE; ++i)
    {
        unsigned len = rand() % 10 + 16;
        unsigned long ts = getCurrentTimeInMillis();
        if (shmwriter.sendData(buff, len, err_msg) == 0)
        {
            ofs << std::string(buff, len) << "+" << ts << std::endl;
        }
        else
        {
            std::cout << err_msg << std::endl;
        }
    }
    ofs.close();
    #endif
    unsigned counter = 0;
    #define SCALE 1000000
    std::string err_msg;
    std::list<unsigned long> tsq;
    unsigned long start_ts = getCurrentTimeInMillis();
    for (int i = 0;i < SCALE; ++i)
    {
        tsq.push_back(getCurrentTimeInMillis());
        if (shmwriter.sendData(buff, 1000, err_msg) == 0)
        {
            counter++;
        }
        else
        {
            tsq.pop_back();
        }
    }
    unsigned long end_ts = getCurrentTimeInMillis();
    std::cout << end_ts - start_ts <<"ms send " << counter << " data" << std::endl;

    FILE *fp = fopen("sender.txt", "w");
    for (std::list<unsigned long>::iterator it = tsq.begin();
        it != tsq.end(); ++it)
        fprintf(fp, "%lu\n", *it);
    fclose(fp);
}
