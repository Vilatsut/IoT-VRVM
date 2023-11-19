#include <stdio.h>

#include "lpsxxx.h"
#include "lpsxxx_params.h"

#include "ztimer.h"
#include "shell.h"
#include "mutex.h"

#define SENSOR_PERIOD_MS    5000
#define INIT_SLEEP          5000

// Global variables
static lpsxxx_t lpsxxx;

static char sensor_stack[THREAD_STACKSIZE_MAIN];

// Mutex to protect global variables
mutex_t sensor_mutex;

static int16_t g_temperature;
static uint16_t g_pressure;

static int sensor_handler(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    mutex_lock(&sensor_mutex);
    printf("Temperature %i.%u°C, Pressure %uhPa \n", g_temperature / 100, 
            g_temperature % 100, g_pressure);
    mutex_unlock(&sensor_mutex);
    return 0;
}

static void* sensor_thread(void *arg)
{
    (void) arg;

    while(1)
    {
        mutex_lock(&sensor_mutex);
        lpsxxx_read_temp(&lpsxxx, &g_temperature);
        lpsxxx_read_pres(&lpsxxx, &g_pressure);
        mutex_unlock(&sensor_mutex);

        ztimer_sleep(ZTIMER_MSEC, SENSOR_PERIOD_MS);
    }

    return NULL; // Should never get there
}

static const shell_command_t shell_commands[] =
{
    { "sensors", "print lps331ap values", sensor_handler },
    { NULL, NULL, NULL},
};

int main(void)
{
    // Just have enough sleep to get the first print also shown to terminal
    ztimer_sleep(ZTIMER_MSEC, INIT_SLEEP);
    puts("Application starts");

    lpsxxx_init(&lpsxxx, &lpsxxx_params[0]);

    thread_create(sensor_stack, sizeof(sensor_stack), THREAD_PRIORITY_MAIN - 1,
                  0, sensor_thread, NULL, "Writer thread");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}