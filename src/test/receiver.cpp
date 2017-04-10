#include <iostream>
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "single_consumer.h"
#include <list>

long int getCurrentTimeInMillis()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}

class Client: public SHM_CALLBACK
{
public:
    Client(int scale)
    {
//        ofs.open ("receiver.txt", std::ofstream::out);
        starttime = 0;
        endtime = 0;
        counter = 0;
        this->scale = scale;
    }

    ~Client()
    {
//        ofs.close();
    }

    void do_poll(Blob_Type *buffer_blob)
    {
//        long int currenttime = getCurrentTimeInMillis();
//        ofs << std::string(buffer_blob->data, buffer_blob->len) << "+" << currenttime << std::endl;
        if (!starttime)
            starttime = getCurrentTimeInMillis();
        endtime = getCurrentTimeInMillis();
        ++counter;
        tsq.push_back(getCurrentTimeInMillis());
        if (counter % scale == 0)
        {
            fp = fopen("receiver.txt", "w");
            std::cout << endtime - starttime << " read " << scale << " data\n";
            for (std::list<unsigned long>::iterator it = tsq.begin();it != tsq.end(); ++it)
                fprintf(fp, "%lu\n", *it);
            fclose(fp);
            starttime = 0;
        }
    }
private:
    FILE* fp;
    std::list<unsigned long> tsq;
    int counter;
    int scale;
    unsigned int starttime;
    unsigned int endtime;
};

int main()
{
    Consumer shmreader("../../conf/test.ini");
    Client client(1000000);

    shmreader.listen(&client);
}
