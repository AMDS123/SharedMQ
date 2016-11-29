#ifndef __SCOPEDFLOCK_HEADER__
#define __SCOPEDFLOCK_HEADER__

#include <fcntl.h>

namespace util
{

class Scoped_Flock
{
public:
    Scoped_Flock(int _fd);
    ~Scoped_Flock();
private:
    int fd;
    struct flock lock;
};

}

#endif