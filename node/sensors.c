#include "lpsxxx.h"
#include "lpsxxx_params.h"
#include "sensors.h"

#define LPS331AP_REG_RES_CONF           0x10
#define LPS331AP_REG_CTRL_REG2          0x21

// Global variables
static lpsxxx_t lpsxxx;

int sensors_init(void)
{
    if (lpsxxx_init(&lpsxxx, &lpsxxx_params[0]) != LPSXXX_OK)
    {
        return 1;
    }

#if 0
    // Gives a software reset for the sensor
    // Tried because sensor gives wrong values

    i2c_acquire(lpsxxx.params.i2c);
    if (i2c_write_reg(lpsxxx.params.i2c, lpsxxx.params.addr, 
                      LPS331AP_REG_CTRL_REG2, 0x84, 0) < 0)
    {
        i2c_release(lpsxxx.params.i2c);
        return 1;
    }
    i2c_release(lpsxxx.params.i2c);
#endif

    if (lpsxxx_enable(&lpsxxx) != LPSXXX_OK)
    {
        return 1;
    }

    return 0;
}

int sensors_get_values(sensor_values_t* values)
{
    if (lpsxxx_read_temp(&lpsxxx, &(values->temperature)) != LPSXXX_OK)
    {
        return 1;
    }

    if (lpsxxx_read_pres(&lpsxxx, &(values->pressure)) != LPSXXX_OK)
    {
       return 1;
    }

    return 0;
}