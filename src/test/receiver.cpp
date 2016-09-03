#include "shmmqcommu.hpp"
#include <iostream>

class Client: public SHM_CALLBACK
{
public:
    void do_poll(Blob_Type *buffer_blob)
    {
        printf("%u\n", buffer_blob->len);
        for (unsigned i = 0;i < buffer_blob->len; ++i)
        {
            std::cout << buffer_blob->data[i];
        }
        std::cout << std::endl;
    }
};

int main()
{
    ShmMQCommu shmmq("/Users/leechanx/Desktop/ShmMQ/conf/test.ini");
    Client client;

    shmmq.listen(&client);
}
