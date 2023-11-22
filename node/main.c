#include <stdio.h>

#include "ztimer.h"
#include "shell.h"
#include "mutex.h"
#include "msg.h"

#include "sensors.h"
#include "coap_wrapper.h"

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];


static int coap_handler(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    return coap_handler_impl();
}

static int sensor_handler(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    sensor_values_t values;
    if (sensors_get_values(&values))
    {
        puts("Failed to get sensor values");
    }

    printf("Temperature %i.%u°C, Pressure %uhPa \n", values.temperature / 100, 
            values.temperature % 100, values.pressure);
    return 0;
}

static const shell_command_t shell_commands[] =
{
    { "sensors", "Print lps331ap values", sensor_handler },
    { "sensor_coap", "Send sensor values as POST request over CoAP to predefined IPv6 address", coap_handler},
    { NULL, NULL, NULL},
};

int main(void)
{
    puts("Application starts");
    
    if (sensors_init())
    {
        puts("Sensors init failed");
    }

    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}