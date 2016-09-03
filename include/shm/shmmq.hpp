#ifndef __SHM_HEADER__
#define __SHM_HEADER__

#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct ShmMQStat
{
    unsigned msg_count;
    unsigned processed_count;
};

class ShmMQ
{
public:
    ShmMQ(const char *path, int id, size_t shm_size);
    ~ShmMQ();

    int enqueue(const void *data, unsigned data_len);
    int dequeue(void *buffer, unsigned buffer_size, unsigned &data_len);
    void debug();

private:
    void init(bool isSender = true);

    int shmid;
    size_t shm_size;
    void *shm_mem;

    unsigned *head_ptr;
    unsigned *tail_ptr;
    ShmMQStat *shm_stat;

    unsigned block_size;
    char *block_ptr;
};

#define BOUND_VALUE 0x58505053
#define BOUND_VALUE_LEN 4

#define MSG_HEAD_LEN (sizeof(unsigned) + BOUND_VALUE_LEN)

#endif