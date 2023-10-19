#ifndef OTHER_H
#define OTHER_H

#include "other.h"

void SleepS(float seconds)
{
    #ifdef __WIN32
        Sleep(seconds * 1000);
    #else
        sleep(seconds);
    #endif
}
#endif
