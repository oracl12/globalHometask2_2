#include "../headers/other.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void SleepSX(float seconds) {
#ifdef _WIN32
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif
}
