#include <stdio.h>

#include "lpsxxx.h"
#include "lpsxxx_params.h"

#include "ztimer.h"

#define SLEEP_TIME_MS   5000

static lpsxxx_t lpsxxx;

static char sensor_stack[THREAD_STACKSIZE_MAIN];

static void* sensor_thread(void *arg)
{
    (void) arg;

    while(1)
    {
        int16_t temp = 0;
        uint16_t pressure = 0;

        lpsxxx_read_temp(&lpsxxx, &temp);
        lpsxxx_read_pres(&lpsxxx, &pressure);

        printf("Temperature %i.%u°C, Pressure %uhPa \n", temp / 100, temp % 100, pressure);

        ztimer_sleep(ZTIMER_MSEC, SLEEP_TIME_MS);
    }

    return NULL; // Should never get there
}

int main(void)
{
    // Just have enough sleep to get the first print also shown to terminal
    ztimer_sleep(ZTIMER_MSEC, SLEEP_TIME_MS);
    puts("Application starts");

    lpsxxx_init(&lpsxxx, &lpsxxx_params[0]);

    thread_create(sensor_stack, sizeof(sensor_stack), THREAD_PRIORITY_MAIN - 1,
                  0, sensor_thread, NULL, "Writer thread");

    return 0;
}