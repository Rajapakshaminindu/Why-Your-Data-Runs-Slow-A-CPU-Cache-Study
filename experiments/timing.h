#ifndef TIMING_H
#define TIMING_H

/*
 * timing.h — shared helper for all experiments
 *
 * get_time_ms() returns the current time in milliseconds.
 * We call it before a loop starts, and after it ends.
 * The difference tells us how long the loop took.
 *
 * Works on Linux and WSL2 (Windows Subsystem for Linux).
 */

#include <time.h>

static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

#endif /* TIMING_H */
