#include "scopedflock.hpp"
#include "errors.hpp"

namespace util
{

Scoped_Flock::Scoped_Flock(int _fd): fd(_fd)
{
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;//区域为0~无穷
    lock.l_type = F_WRLCK;
    exit_if(fcntl(fd, F_SETLKW, &lock) != 0, "fcntl lock");
}

Scoped_Flock::~Scoped_Flock()
{
    lock.l_type = F_UNLCK;
    exit_if(fcntl(fd, F_SETLK, &lock) != 0, "fcntl unlock");
}

}
