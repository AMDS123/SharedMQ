#include <unistd.h>
#include <iostream>
#include "shmmq.h"
#include "errors.h"
#include "configreader.h"

namespace ShmBase
{

ShmMQ::ShmMQ(const char *conf_path)
{
    const char *key_path = util::ConfigReader::getConfigReader(conf_path)->GetString("shm", "keypath", "").c_str();
    int id = util::ConfigReader::getConfigReader(conf_path)->GetNumber("shm", "id", 1);
    unsigned shm_size = util::ConfigReader::getConfigReader(conf_path)->GetNumber("shm", "shmsize", 10240);

    key_t key = ::ftok(key_path, id);
    exit_if(key == (key_t)-1, "ftok key");

    bool first_attach = false;

    int shmid;
    if ((shmid = ::shmget(key, shm_size, 0666)) == -1)
    {
        shmid = ::shmget(key, shm_size, IPC_CREAT | 0666);
        first_attach = true;
        exit_if(shmid == -1, "shmget");
    }

    shm_mem = ::shmat(shmid, NULL, 0);
    exit_if(shm_mem == NULL, "shmat");

    head_ptr = (unsigned *)shm_mem;
    tail_ptr = head_ptr + 1;

    block_ptr = (char *)(tail_ptr + 1);
    block_size = shm_size - sizeof(unsigned) * 2;

    if (first_attach)
    {
        *head_ptr = 0;
        *tail_ptr = 0;
    }

    short number = 0x0001;
    char *p = (char *)&number;
    endian_solution = *p == 0x00? BIG_ENDIAN_VALUE: LITTLE_ENDIAN_VALUE;
}

ShmMQ::~ShmMQ()
{
    ::shmdt(shm_mem);
}

int ShmMQ::enqueue(const void *data, unsigned data_len, std::string& err_msg)
{
    unsigned head = *head_ptr, tail = *tail_ptr;
    unsigned free_len = head <= tail ? block_size - tail + head: head - tail;
    unsigned tail_2_end_len = block_size - tail;
    unsigned total_len = MSG_HEAD_LEN + data_len + BOUND_VALUE_LEN;
    unsigned new_tail_addr = 0;

    //Note: should leave 1 Byte to be a sentinel
    //if not, we can't know that head = tail is empty or full...
    //head = tail: empty
    //head = (tail + 1) % size: full
    if (total_len >= free_len)
    {
        err_msg = "no space to enqueue data";
        return QUEUE_ERR_FULL;
    }

    char msg_head[MSG_HEAD_LEN] = {};
    *((unsigned *)msg_head) = BEGIN_BOUND_VALUE;
    memcpy(msg_head + BOUND_VALUE_LEN, &total_len, sizeof(unsigned));

    if (tail_2_end_len >= total_len)
    {
        memcpy(block_ptr + tail, msg_head, MSG_HEAD_LEN);
        memcpy(block_ptr + tail + MSG_HEAD_LEN, data, data_len);
        *((unsigned *)(block_ptr + tail + MSG_HEAD_LEN + data_len)) = END_BOUND_VALUE;
        new_tail_addr = tail + total_len;
    }
    //tail_2_end_len < total_len
    else if (tail_2_end_len >= MSG_HEAD_LEN)
    {
        memcpy(block_ptr + tail, msg_head, MSG_HEAD_LEN);
        unsigned first_data_len = tail_2_end_len - MSG_HEAD_LEN;
        unsigned second_data_len = data_len + BOUND_VALUE_LEN - first_data_len;
        if (second_data_len >= BOUND_VALUE_LEN)
        {
            memcpy(block_ptr + tail + MSG_HEAD_LEN, data, first_data_len);
            memcpy(block_ptr, (char *)data + first_data_len, data_len - first_data_len);
            *((unsigned *)(block_ptr + data_len - first_data_len)) = END_BOUND_VALUE;
        }
        else
        {
            memcpy(block_ptr + tail + MSG_HEAD_LEN, data, data_len);
            if (endian_solution == LITTLE_ENDIAN_VALUE)
            {
                switch (second_data_len)
                {
                    case 1:
                        *(block_ptr + block_size - 3) = '=';
                        *(block_ptr + block_size - 2) = 'N';
                        *(block_ptr + block_size - 1) = 'D';
                        *block_ptr = '$';
                        break;
                    case 2:
                        *(block_ptr + block_size - 2) = '=';
                        *(block_ptr + block_size - 1) = 'N';
                        *block_ptr = 'D';
                        *(block_ptr + 1) = '$';
                        break;
                    case 3:
                        *(block_ptr + block_size - 1) = '=';
                        *block_ptr = 'N';
                        *(block_ptr + 1) = 'D';
                        *(block_ptr + 2) = '$';
                }
            }
            else
            {
                switch (second_data_len)
                {
                    case 1:
                        *(block_ptr + block_size - 3) = '$';
                        *(block_ptr + block_size - 2) = 'D';
                        *(block_ptr + block_size - 1) = 'N';
                        *block_ptr = '=';
                        break;
                    case 2:
                        *(block_ptr + block_size - 2) = '$';
                        *(block_ptr + block_size - 1) = 'D';
                        *block_ptr = 'N';
                        *(block_ptr + 1) = '=';
                        break;
                    case 3:
                        *(block_ptr + block_size - 1) = '$';
                        *block_ptr = 'D';
                        *(block_ptr + 1) = 'N';
                        *(block_ptr + 2) = '=';
                }
            }
        }
        new_tail_addr = second_data_len;
    }
    //tail_2_end_len < MSG_HEAD_LEN
    else
    {
        memcpy(block_ptr + tail, msg_head, tail_2_end_len);
        unsigned leave_msg_head_len = MSG_HEAD_LEN - tail_2_end_len;
        memcpy(block_ptr, msg_head + tail_2_end_len, leave_msg_head_len);
        memcpy(block_ptr + leave_msg_head_len, data, data_len);
        *((unsigned *)(block_ptr + leave_msg_head_len + data_len)) = END_BOUND_VALUE;
        new_tail_addr = leave_msg_head_len + data_len + BOUND_VALUE_LEN;//tail
    }
    *tail_ptr = new_tail_addr;//update tail addr
    err_msg = "";
    return QUEUE_SUCC;
}

int ShmMQ::dequeue(void *buffer, unsigned buffer_size, unsigned &data_len, std::string& err_msg)
{
    unsigned head = *head_ptr, tail = *tail_ptr;
    std::cout << "head:" << head << " tail:" << tail << std::endl;
    if (head == tail)
    {
        err_msg = "shm empty";
        return QUEUE_ERR_EMPTY;
    }

    unsigned new_head_addr = 0;
    unsigned used_len = head < tail ? tail - head: block_size + tail - head;
    //copy msg_head out first
    char msg_head[MSG_HEAD_LEN] = {};
    //msg_head is in [head, ...)
    if (head + MSG_HEAD_LEN <= block_size)
    {
        memcpy(msg_head, block_ptr + head, MSG_HEAD_LEN);
        head += MSG_HEAD_LEN;
    }
    //msg_head is sub in [*block_ptr, tail) and sub in [head, ...)
    else
    {
        unsigned first_msg_head_len = block_size - head;
        unsigned second_msg_head_len = MSG_HEAD_LEN - first_msg_head_len;
        memcpy(msg_head, block_ptr + head, first_msg_head_len);
        memcpy(msg_head + first_msg_head_len, block_ptr, second_msg_head_len);
        head = second_msg_head_len;
    }
    //copy real data now
    unsigned sentinel_head =  *(unsigned *)msg_head;
    unsigned total_len = *((unsigned *)(msg_head + BOUND_VALUE_LEN));
    if (sentinel_head != BEGIN_BOUND_VALUE)
    {
        err_msg = "sentinel check error.";
        return QUEUE_ERR_CHECKSEN;
    }
    if (total_len > used_len)
    {
        err_msg = "mem is messed up.";
        return QUEUE_ERR_MEMESS;
    }
    data_len = total_len - MSG_HEAD_LEN;
    if (data_len > buffer_size)
    {
        err_msg = "user buffer overflow.";
        return QUEUE_ERR_OTFBUFF;
    }
    //data is in [head, ...)
    if (head + data_len <= block_size)
    {
        memcpy(buffer, block_ptr + head, data_len);
        new_head_addr = head + data_len;
    }
    //data is sub in [*block_ptr, tail) and sub in [head, ...)
    else
    {
        unsigned first_data_len = block_size - head;
        unsigned second_data_len = data_len - first_data_len;
        memcpy(buffer, block_ptr + head, first_data_len);
        memcpy((char *)buffer + first_data_len, block_ptr, second_data_len);
        new_head_addr = second_data_len;
    }
    data_len -= BOUND_VALUE_LEN;
    unsigned sentinel_tail = *((unsigned *)((char *)buffer + data_len));
    if (sentinel_tail != END_BOUND_VALUE)
    {
        err_msg = "sentinel check error.";
        return QUEUE_ERR_CHECKSEN;
    }
    *head_ptr = new_head_addr;//update head addr
    return QUEUE_SUCC;
}

}
