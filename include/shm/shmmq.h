#ifndef __SHM_HEADER__
#define __SHM_HEADER__

#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <string>

namespace ShmBase
{

#define BIG_ENDIAN_VALUE 0
#define LITTLE_ENDIAN_VALUE 1
#define BEGIN_BOUND_VALUE 0x3d45425e//little-endian of: "^BE="
#define END_BOUND_VALUE 0x24444e3d//little-endian of: "=ND$"
#define BOUND_VALUE_LEN 4
#define MSG_HEAD_LEN (sizeof(unsigned) + BOUND_VALUE_LEN)

class ShmMQ
{
public:
    ShmMQ(const char *conf_path);
    virtual ~ShmMQ();

    /*
     enqueue: push data to shmmq
     data: write content
     data_len: write content size
     err_msg: error message
    */
    int enqueue(const void *data, unsigned data_len, std::string& err_msg);

    /*
     dequeue: pop data from shmmq
     buffer: read buffer
     buffer_size: read buffer size
     data_len: data length (return)
     err_msg: error message
    */
    int dequeue(void *buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg);

private:
    void init(bool first_use);

    char endian_solution;//machine endian

    /* shm format

      shm_mem = |head|tail|data block|

      data block is a ring queue
      head is queue's head
      tail is queue's tail
    */
    void *shm_mem;
    char *block_ptr;//data block head ptr
    unsigned block_size;//data block size

    unsigned *head_ptr;//shm head
    unsigned *tail_ptr;//shm tail
};

}

#endif