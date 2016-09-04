#include "shmmq.hpp"

void ShmMQ::init(bool isSender)
{
    if ((shm_mem = shmat(shmid, NULL, 0)) == NULL)
    {
        perror("shmget error.");
        exit(1);
    }

    shm_stat = (ShmMQStat *)shm_mem;
    shm_mem = (void *)((char *)shm_mem + sizeof(ShmMQStat));
    
    head_ptr = (unsigned *)shm_mem;

    tail_ptr = head_ptr + 1;

    block_ptr = (char *)(tail_ptr + 1);
    block_size = shm_size - sizeof(unsigned) * 2 - sizeof(ShmMQStat);

    if (isSender)
    {
        *head_ptr = 0;
        *tail_ptr = 0;
    }
}

ShmMQ::ShmMQ(const char *path, int id, size_t shm_size)
{
    key_t key = ftok(path, id);
    if (key == (key_t)-1)
    {
        perror("ftok key error.");
        exit(1);
    }
    
    this->shm_size = shm_size;
    printf("what hell %lu\n", shm_size);

    if ((shmid = shmget(key, shm_size, 0666)) == -1)
    {
        if ((shmid = shmget(key, shm_size, IPC_CREAT | 0666)) == -1)
        {
            perror("shmget error.");
            exit(1);
        }
        else
        {
            //I am sender.
            printf("I am sender.\n");
            init();
        }
    }
    else
    {
        //I am receiver.
        printf("I am receiver.\n");
        init(false);
    }
}

ShmMQ::~ShmMQ()
{
    /*
    int ret;
    if ((ret = shmctl(shmid, IPC_RMID, 0)) < 0)
    {
        perror("shmctl error.");
        exit(1);
    }
    */
}

void ShmMQ::debug()
{
    unsigned head = *head_ptr, tail = *tail_ptr;
    printf("head = %u, tail = %u\n", head, tail);
}

#define BOUND_VALUE 0x58505053
#define BOUND_VALUE_LEN 4

#define MSG_HEAD_LEN (sizeof(unsigned) + BOUND_VALUE_LEN)

int ShmMQ::enqueue(const void *data, unsigned data_len)
{
    if (data_len == 0)
    {
        perror("fuck off.");
        return -1;
    }
    unsigned head = *head_ptr, tail = *tail_ptr;

    unsigned free_len = head <= tail ? block_size - tail + head: head - tail;
    unsigned tail_2_end_len = block_size - tail;

    unsigned total_len = MSG_HEAD_LEN + data_len + BOUND_VALUE_LEN;

    //Note: should leave 1 Byte to be a sentinel
    //if not, we can't know that head = tail is empty or full...
    //head = tail: empty
    //head = tail + 1: full
    if (total_len >= free_len)
    {
        perror("no space to enqueue data");
        exit(1);
    }

    char msg_head[MSG_HEAD_LEN] = {};
    *((unsigned *)msg_head) = BOUND_VALUE;
    memcpy(msg_head + BOUND_VALUE_LEN, &total_len, sizeof(unsigned));

    if (tail_2_end_len >= total_len)
    {
        memcpy(block_ptr + tail, msg_head, MSG_HEAD_LEN);
        memcpy(block_ptr + tail + MSG_HEAD_LEN, data, data_len);
        *((unsigned *)(block_ptr + tail + MSG_HEAD_LEN + data_len)) = BOUND_VALUE;
        *tail_ptr += total_len;
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
            *((unsigned *)(block_ptr + data_len - first_data_len)) = BOUND_VALUE;
        }
        else
        {
            memcpy(block_ptr + tail + MSG_HEAD_LEN, data, data_len);
            switch (second_data_len)
            {
                case 1:
                    *(block_ptr + block_size - 3) = 'S';
                    *(block_ptr + block_size - 2) = 'P';
                    *(block_ptr + block_size - 1) = 'P';
                    *block_ptr = 'X';
                    break;
                case 2:
                    *(block_ptr + block_size - 2) = 'S';
                    *(block_ptr + block_size - 1) = 'P';
                    *block_ptr = 'P';
                    *(block_ptr + 1) = 'X';
                    break;
                case 3:
                    *(block_ptr + block_size - 1) = 'S';
                    *block_ptr = 'P';
                    *(block_ptr + 1) = 'P';
                    *(block_ptr + 2) = 'X';
                    break;
                default:
                    perror("what the fuck is happenning?");
                    exit(1);
            }
        }
        *tail_ptr = second_data_len;
    }
    //tail_2_end_len < MSG_HEAD_LEN
    else
    {
        memcpy(block_ptr + tail, msg_head, tail_2_end_len);
        unsigned leave_msg_head_len = MSG_HEAD_LEN - tail_2_end_len;
        memcpy(block_ptr, msg_head + tail_2_end_len, leave_msg_head_len);
        memcpy(block_ptr + leave_msg_head_len, data, data_len);
        *((unsigned *)(block_ptr + leave_msg_head_len + data_len)) = BOUND_VALUE;
        *tail_ptr = leave_msg_head_len + data_len + BOUND_VALUE_LEN;
    }
    return 1;
}

int ShmMQ::dequeue(void *buffer, unsigned buffer_size, unsigned &data_len)
{
    unsigned head = *head_ptr, tail = *tail_ptr;
    if (head == tail)
    {
        perror("shm empty error.");
        exit(1);
    }
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
    if (sentinel_head != BOUND_VALUE)
    {
        perror("sentinel check error.");
        exit(1);
    }
    if (total_len > used_len)
    {
        perror("mem is fuck up.");
        exit(1);
    }
    data_len = total_len - MSG_HEAD_LEN;
    if (data_len > buffer_size)
    {
        perror("holly shit...");
        exit(1);
    }

    //data is in [head, ...)
    if (head + data_len < block_size)
    {
        memcpy(buffer, block_ptr + head, data_len);
        *head_ptr = head + data_len;
    }
    //data is sub in [*block_ptr, tail) and sub in [head, ...)
    else
    {
        unsigned first_data_len = block_size - head;
        unsigned second_data_len = data_len - first_data_len;
        memcpy(buffer, block_ptr + head, first_data_len);
        memcpy((char *)buffer + first_data_len, block_ptr, second_data_len);
        *head_ptr = second_data_len;
    }
    data_len -= BOUND_VALUE_LEN;
    unsigned sentinel_tail = *((unsigned *)((char *)buffer + data_len));
    if (sentinel_tail != BOUND_VALUE)
    {
        perror("sentinel check error.");
        exit(1);
    }
    return 1;
}
