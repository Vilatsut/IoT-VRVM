#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

typedef struct {
    int16_t temperature;
    uint16_t pressure;
} sensor_values_t;

int sensors_init(void);
sensor_values_t sensors_get_values(void);

#endif /* SENSORS_H */