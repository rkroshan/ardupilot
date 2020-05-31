#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/system.h>
#include <AP_HAL_Linux/Scheduler.h>

extern const AP_HAL::HAL& hal;

namespace AP_HAL {

static struct {
    struct timespec start_time;
} state;

void init()
{
    clock_gettime(CLOCK_MONOTONIC, &state.start_time);
}

void panic(const char *errormsg, ...)
{
    va_list ap;

    va_start(ap, errormsg);
    vdprintf(1, errormsg, ap);
    va_end(ap);
    UNUSED_RESULT(write(1, "\n", 1));

    if (hal.rcin != nullptr) {
        hal.rcin->teardown();
    }
    if (hal.scheduler != nullptr) {
        hal.scheduler->delay_microseconds(10000);
    }
    exit(1);
}

uint32_t micros()
{
    return micros64() & 0xFFFFFFFF;
}

uint32_t millis()
{
    return millis64() & 0xFFFFFFFF;
}

uint64_t micros64()
{
    const Linux::Scheduler* scheduler = Linux::Scheduler::from(hal.scheduler);
    uint64_t stopped_usec = scheduler->stopped_clock_usec();
    if (stopped_usec) {
        return stopped_usec;
    }

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return 1.0e6*((ts.tv_sec + (ts.tv_nsec*1.0e-9)) -
                  (state.start_time.tv_sec +
                   (state.start_time.tv_nsec*1.0e-9)));
}

uint64_t millis64()
{
    const Linux::Scheduler* scheduler = Linux::Scheduler::from(hal.scheduler);
    uint64_t stopped_usec = scheduler->stopped_clock_usec();
    if (stopped_usec) {
        return stopped_usec / 1000;
    }

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return 1.0e3*((ts.tv_sec + (ts.tv_nsec*1.0e-9)) -
                  (state.start_time.tv_sec +
                   (state.start_time.tv_nsec*1.0e-9)));
}


uint32_t native_micros()
{
    return native_micros64() & 0xFFFFFFFF;
}

uint32_t native_millis()
{
    return native_millis64() & 0xFFFFFFFF;
}

/*
  we define a millis16() here to avoid an issue with sitl builds in cygwin
 */
uint16_t native_millis16()
{
    return native_millis64() & 0xFFFF;
}
    

uint64_t native_micros64()
{
    struct timeval tp;
    gettimeofday(&tp, nullptr);
    uint64_t ret = 1.0e6 * ((tp.tv_sec + (tp.tv_usec * 1.0e-6)) -
                            (state.start_time.tv_sec +
                             (state.start_time.tv_nsec*1.0e-9)));
    return ret;
}

uint64_t native_millis64()
{
    struct timeval tp;
    gettimeofday(&tp, nullptr);
    uint64_t ret = 1.0e3*((tp.tv_sec + (tp.tv_usec*1.0e-6)) -
                          (state.start_time.tv_sec +
                           (state.start_time.tv_nsec*1.0e-9)));
    return ret;
}


} // namespace AP_HAL
