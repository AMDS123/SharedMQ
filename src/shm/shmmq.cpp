#include "shmmq.hpp"
#include "errors.hpp"
#include "configreader.hpp"
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>

void ShmMQ::init(Role role)
{
    shm_mem = shmat(shmid, NULL, 0);
    exit_if(shm_mem == NULL, "shmat");

    shm_stat = (ShmMQStat *)shm_mem;
    shm_mem = (void *)((char *)shm_mem + sizeof(ShmMQStat));

    head_ptr = (unsigned *)shm_mem;

    tail_ptr = head_ptr + 1;

    block_ptr = (char *)(tail_ptr + 1);
    block_size = shm_size - sizeof(unsigned) * 2 - sizeof(ShmMQStat);

    if (role == WRITER)
    {
        *head_ptr = 0;
        *tail_ptr = 0;

        if (access("/tmp/shmmq", F_OK) == -1)
        {
            if (mkdir("/tmp/shmmq", S_IREAD | S_IWRITE | S_IEXEC) == -1)
            {
                TELL_ERROR("mkdir for /tmp/shmmq");
            }
        }

        std::ofstream ofs;
        ofs.open("/tmp/shmmq/shmid", std::ofstream::out);
        ofs << shmid;
        ofs.close();
    }
}

ShmMQ::ShmMQ(const char *conf_path, Role role)
{
    const char *key_path = ConfigReader::getConfigReader(conf_path)->GetString("shm", "keypath", "").c_str();
    int id = ConfigReader::getConfigReader(conf_path)->GetNumber("shm", "id", 1);
    unsigned shm_size = ConfigReader::getConfigReader(conf_path)->GetNumber("shm", "shmsize", 10240);

    key_t key = ftok(key_path, id);
    exit_if(key == (key_t)-1, "ftok key");
    
    this->shm_size = shm_size;

    if ((shmid = shmget(key, shm_size, 0666)) == -1)
    {
        shmid = shmget(key, shm_size, IPC_CREAT | 0666);
        exit_if(shmid == -1, "shmget");
    }
    init(role);
    set_endian();
}

ShmMQ::~ShmMQ()
{
}

int ShmMQ::enqueue(const void *data, unsigned data_len)
{
    unsigned head = *head_ptr, tail = *tail_ptr;   
    if (!do_check(head, tail))
    {
        TELL_ERROR("head: %u, tail: %u, check fail.", head, tail);
        return QUEUE_ERR_CHECKHT;
    }

    unsigned free_len = head <= tail ? block_size - tail + head: head - tail;
    unsigned tail_2_end_len = block_size - tail;

    unsigned total_len = MSG_HEAD_LEN + data_len + BOUND_VALUE_LEN;

    //Note: should leave 1 Byte to be a sentinel
    //if not, we can't know that head = tail is empty or full...
    //head = tail: empty
    //head = (tail + 1) % size: full
    if (total_len >= free_len)
    {
        TELL_ERROR("no space to enqueue data");
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
                    break;
                default:
                    TELL_ERROR("impossbile");
                    exit(1);
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
                        break;
                    default:
                        TELL_ERROR("impossbile");
                        exit(1);
                }
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
        *((unsigned *)(block_ptr + leave_msg_head_len + data_len)) = END_BOUND_VALUE;
        *tail_ptr = leave_msg_head_len + data_len + BOUND_VALUE_LEN;
    }
    return 0;
}

void ShmMQ::clear()
{
    *head_ptr = 0;
    *tail_ptr = 0;
    //ShmMQStat need to be updated.
    TELL_ERROR("clear shm.");
}

bool ShmMQ::is_begin_bound(unsigned pos)
{
    if (endian_solution == LITTLE_ENDIAN_VALUE)
    {
        if (*(block_ptr + pos) == '^' &&
        *(block_ptr + ((pos + 1) % block_size)) == 'B' &&
        *(block_ptr + ((pos + 2) % block_size)) == 'E' &&
        *(block_ptr + ((pos + 3) % block_size)) == '=')
        {
            return true;
        }
    }
    else
    {
        if (*(block_ptr + pos) == '=' &&
        *(block_ptr + ((pos + 1) % block_size)) == 'E' &&
        *(block_ptr + ((pos + 2) % block_size)) == 'B' &&
        *(block_ptr + ((pos + 3) % block_size)) == '^')
        {
            return true;
        }
    }
    return false;
}

bool ShmMQ::is_end_bound(unsigned pos)
{
    if (endian_solution == LITTLE_ENDIAN_VALUE)
    {
        if (*(block_ptr + pos) == '=' &&
        *(block_ptr + ((pos + 1) % block_size)) == 'N' &&
        *(block_ptr + ((pos + 2) % block_size)) == 'D' &&
        *(block_ptr + ((pos + 3) % block_size)) == '$')
        {
            return true;
        }
    }
    else
    {
        if (*(block_ptr + pos) == '$' &&
        *(block_ptr + ((pos + 1) % block_size)) == 'D' &&
        *(block_ptr + ((pos + 2) % block_size)) == 'N' &&
        *(block_ptr + ((pos + 3) % block_size)) == '=')
        {
            return true;
        }
    }
    return false;
}

void ShmMQ::reset()
{
    TELL_ERROR("reset shm.");
    unsigned head = *head_ptr, tail = *tail_ptr;
    if (head < tail)
    {
        while (tail - head > MSG_HEAD_LEN + BOUND_VALUE_LEN)
        {
            if (is_begin_bound(head))
            {
                char msg_head[MSG_HEAD_LEN] = {};
                memcpy(msg_head, block_ptr + head, MSG_HEAD_LEN);
                unsigned data_len = *((unsigned *)(msg_head + BOUND_VALUE_LEN)) - MSG_HEAD_LEN - BOUND_VALUE_LEN;

                unsigned pos = head + MSG_HEAD_LEN + data_len;
                if (pos + BOUND_VALUE_LEN > tail)
                {
                    break;
                }
                if (is_end_bound(pos))
                {
                    *head_ptr = head;
                    TELL_ERROR("reset *head_ptr = %u", head);
                    return ;
                }
            }
            else
            {
                ++head;
            }
        }
        clear();
    }
    else if (head > tail)
    {
        while (head < block_size)
        {
            if (block_size - head + tail <= MSG_HEAD_LEN + BOUND_VALUE_LEN)
            {
                clear();
                return ;
            }
            if (is_begin_bound(head))
            {
                char msg_head[MSG_HEAD_LEN] = {};
                if (head + MSG_HEAD_LEN <= block_size)
                {
                    memcpy(msg_head, block_ptr + head, MSG_HEAD_LEN);
                }
                else
                {
                    unsigned first_msg_head_len = block_size - head;
                    unsigned second_msg_head_len = MSG_HEAD_LEN - first_msg_head_len;
                    memcpy(msg_head, block_ptr + head, first_msg_head_len);
                    memcpy(msg_head + first_msg_head_len, block_ptr, second_msg_head_len);
                }

                unsigned data_len = *((unsigned *)(msg_head + BOUND_VALUE_LEN)) - MSG_HEAD_LEN - BOUND_VALUE_LEN;
                unsigned pos = (head + MSG_HEAD_LEN + data_len) % block_size;
                if ((pos + BOUND_VALUE_LEN) % block_size > tail)
                {
                    clear();
                    return ;
                }
                if (is_end_bound(pos))
                {
                    *head_ptr = head;
                    TELL_ERROR("reset *head_ptr = %u", head);
                    return ;
                }
            }
        }
        head = 0;
        while (tail - head > MSG_HEAD_LEN + BOUND_VALUE_LEN)
        {
            if (is_begin_bound(head))
            {
                char msg_head[MSG_HEAD_LEN] = {};
                memcpy(msg_head, block_ptr + head, MSG_HEAD_LEN);
                unsigned data_len = *((unsigned *)(msg_head + BOUND_VALUE_LEN)) - MSG_HEAD_LEN - BOUND_VALUE_LEN;

                unsigned pos = head + data_len;
                if (pos + BOUND_VALUE_LEN > tail)
                {
                    break;
                }
                if (is_end_bound(pos))
                {
                    *head_ptr = head;
                    TELL_ERROR("reset *head_ptr = %u", head);
                    return ;
                }
            }
            else
            {
                ++head;
            }
        }
        clear();
    }
}

int ShmMQ::peek(void *buffer, unsigned buffer_size, unsigned &data_len)
{
    unsigned head = *head_ptr, tail = *tail_ptr;
    new_head_addr = 0;
    if (!do_check(head, tail))
    {
        TELL_ERROR("head: %u, tail: %u, check fail.", head, tail);
        return QUEUE_ERR_CHECKHT;
    }
    if (head == tail)
    {
        TELL_ERROR("shm empty");
        return QUEUE_ERR_EMPTY;
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
    if (sentinel_head != BEGIN_BOUND_VALUE)
    {
        TELL_ERROR("sentinel check error.");
        return QUEUE_ERR_CHECKSEN;
    }
    if (total_len > used_len)
    {
        TELL_ERROR("mem is messed up.");
        clear(); //clear shm
        return QUEUE_ERR_MEMESS;
    }
    data_len = total_len - MSG_HEAD_LEN;
    if (data_len > buffer_size)
    {
        TELL_ERROR("user buffer overflow.");
        return QUEUE_ERR_OTFBUFF;
    }

    //data is in [head, ...)
    if (head + data_len < block_size)
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
        TELL_ERROR("sentinel check error.");
        new_head_addr = 0;
        return QUEUE_ERR_CHECKSEN;
    }
    return 0;
}

void ShmMQ::remove(void)
{
    if (new_head_addr)
    {
        *head_ptr = new_head_addr;
    }
}

int ShmMQ::dequeue(void *buffer, unsigned buffer_size, unsigned &data_len)//no use now...
{
    int ret = peek(buffer, buffer_size, data_len);
    if (ret == 0)
    {
        remove();
    }
    return ret;
}
