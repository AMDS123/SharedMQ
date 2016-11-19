#ifndef __SHM_HEADER__
#define __SHM_HEADER__

#include "notify.hpp"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>

struct ShmMQStat
{
    unsigned msg_count;
    unsigned processed_count;
};

class ShmMQ
{
public:
    ShmMQ(const char *conf_path, Role role);
    ~ShmMQ();

    int enqueue(const void *data, unsigned data_len);
    /*  USAGE to get data from shm:
    ----------------------------------------------
        peek(buffer, buffer_size, data_len);
        handle data
        remove();
    ----------------------------------------------
        OR:
        dequeue(buffer, buffer_size, data_len);
        handle data
    ----------------------------------------------
    */
    int peek(void *buffer, unsigned buffer_size, unsigned &data_len);
    void remove();
    int dequeue(void *buffer, unsigned buffer_size, unsigned &data_len);//no use now...

private:
    void init(Role role);

    bool do_check(unsigned head, unsigned tail)
    {
        return head < block_size && tail < block_size;
    }

    int shmid;
    size_t shm_size;
    void *shm_mem;

    unsigned *head_ptr;
    unsigned *tail_ptr;
    ShmMQStat *shm_stat;

    unsigned block_size;
    char *block_ptr;

    unsigned new_head_addr;
};

#define BOUND_VALUE 0x58505053
#define BOUND_VALUE_LEN 4

#define MSG_HEAD_LEN (sizeof(unsigned) + BOUND_VALUE_LEN)

#endif