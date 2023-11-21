#include <stdio.h>

#include "lpsxxx.h"
#include "lpsxxx_params.h"

#include "ztimer.h"
#include "shell.h"
#include "mutex.h"
#include "msg.h"

// Global variables
static lpsxxx_t lpsxxx;

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static int sensor_handler(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    int16_t temperature = 0;
    uint16_t pressure = 0;

    if (lpsxxx_read_temp(&lpsxxx, &temperature) != LPSXXX_OK)
    {
        puts("LPS331AP temperature read failed");
    }

    if (lpsxxx_read_pres(&lpsxxx, &pressure) != LPSXXX_OK)
    {
       puts("LPS331AP pressure read failed");
    }

    printf("Temperature %i.%u°C, Pressure %uhPa \n", temperature / 100, 
            temperature % 100, pressure);
    return 0;
}

static const shell_command_t shell_commands[] =
{
    { "sensors", "print lps331ap values", sensor_handler },
    { NULL, NULL, NULL},
};

int main(void)
{
    puts("Application starts");

    if (lpsxxx_init(&lpsxxx, &lpsxxx_params[0]) != LPSXXX_OK)
    {
        puts("LPS331AP init failed");
    }
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}