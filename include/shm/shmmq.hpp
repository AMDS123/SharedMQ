#ifndef __SHM_HEADER__
#define __SHM_HEADER__

#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>

#define BIG_ENDIAN_VALUE 0
#define LITTLE_ENDIAN_VALUE 1

#define BEGIN_BOUND_VALUE 0x3d45425e//little-endian of: "^BE="
#define END_BOUND_VALUE 0x24444e3d//little-endian of: "=ND$"
#define BOUND_VALUE_LEN 4

#define MSG_HEAD_LEN (sizeof(unsigned) + BOUND_VALUE_LEN)

struct ShmMQStat
{
    unsigned long flag;
};

class ShmMQ
{
public:
    ShmMQ(const char *conf_path);
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
    void remove(void);
    int dequeue(void *buffer, unsigned buffer_size, unsigned &data_len);//no use now...

private:
    void init(bool first_use);

    bool do_check(unsigned head, unsigned tail)
    {
        return head < block_size && tail < block_size;
    }

    void set_endian()
    {
        short number = 0x0001;
        char *p = (char *)&number;
        endian_solution = *p == 0x00? BIG_ENDIAN_VALUE: LITTLE_ENDIAN_VALUE;
    }

    void clear();

    bool is_begin_bound(unsigned pos);
    bool is_end_bound(unsigned pos);

    void reset();

    char endian_solution;

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

#endif