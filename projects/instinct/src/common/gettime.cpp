/*
 * Get time in milliseconds since the initial call.
 */

#ifdef UNIX
#include <sys/time.h>
#else
#include <windows.h>
#endif
#include "common.h"

TIME gettime()
{
    static TIME base_time = 0;
    TIME t;
    #ifdef UNIX
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (base_time == 0)
    {
        base_time = tv.tv_sec;
        return 0;
    }
    else
    {
        t = tv.tv_sec;
        assert(t >= base_time);
        return ((t - base_time) * 1000) + (tv.tv_usec / 1000);
    }
    #else
    if (base_time == 0)
    {
        base_time = (TIME)GetTickCount();
        return 0;
    }
    else
    {
        t = (TIME)GetTickCount();
        assert(t >= base_time);
        return t - base_time;
    }
    #endif
}
