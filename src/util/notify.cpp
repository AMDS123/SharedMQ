#include "notify.hpp"
#include "errors.hpp"
#include "configreader.hpp"

void HANDLE_SIGPIPE(int signo)
{
	TELL_ERROR("receive SIGPIPE signal!");
}

FifoFd::FifoFd(const char *conf_path, Role role)
{
    const char *fifo_path = ConfigReader::getConfigReader(conf_path)->GetString("fifo", "fifopath", "").c_str();
    if (access(fifo_path, F_OK) == -1)
    {
        int ret = mkfifo(fifo_path, 0666);
        exit_if(ret < 0, "mkfifo");
    }

    if (role == READER)
    {
        notify_fd = open(fifo_path, O_RDONLY, 0666);
    }
    else if (role == WRITER)
    {
    	notify_fd = open(fifo_path, O_WRONLY, 0666);
        signal(SIGPIPE, HANDLE_SIGPIPE);
    }    
    exit_if(notify_fd == -1, "open fifo");
}

int FifoFd::notify_event()
{
	int wn;
    if((wn = write(notify_fd, "!", 1)) < 0)
    {
        TELL_ERROR("write fifo error.");
    }
    return wn;
}

int FifoFd::receive_event()
{
    int rn;
    char temp_buffer;
    if ((rn = read(notify_fd, &temp_buffer, 1)) < 0)
    {
        TELL_ERROR("read notify error.");
    }
    return rn;
}