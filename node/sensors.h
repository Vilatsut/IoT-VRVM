#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

typedef struct {
    int16_t temperature;
    uint16_t pressure;
} sensor_values_t;

int sensors_init(void);
int sensors_get_values(sensor_values_t* values);

#endif /* SENSORS_H */