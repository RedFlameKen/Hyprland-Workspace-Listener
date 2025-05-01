#include "sleep.h"
#include <sys/cdefs.h>
#include <time.h>

int8_t sleep_millis(uint32_t ms) {
    struct timespec ts;
    uint8_t ret;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    ret = nanosleep(&ts, &ts);

    return ret;
}
