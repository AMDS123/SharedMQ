#include "errors.hpp"

void exit_if(int condition, const char *format, ...)
{
    va_list arglist;

    if (condition)
    {
        va_start(arglist, format);
        fprintf(stderr, "EXIT BECAUSE OF ERROR: ");
        vfprintf(stderr, format, arglist);
        fprintf(stderr, ": %s\n", strerror(errno));
        va_end(arglist);

        exit(EXIT_FAILURE);   
    }
}
