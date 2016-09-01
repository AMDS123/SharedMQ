#include "shmmqcommu.hpp"
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <time.h>

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
    Client()
    {
        ofs.open ("receiver.txt", std::ofstream::out);
    }

    ~Client()
    {
        ofs.close();
    }

    void do_poll(Blob_Type *buffer_blob)
    {
        std::cout << "receive data" << std::endl;
        long int currenttime = getCurrentTimeInMillis();
        ofs << std::string(buffer_blob->data, buffer_blob->len) << "+" << currenttime << std::endl;
    }
private:
    std::ofstream ofs;
};

int main()
{
    ShmMQCommu shmmq("../../conf/test.ini");
    Client client;

    shmmq.listen(&client);
}
