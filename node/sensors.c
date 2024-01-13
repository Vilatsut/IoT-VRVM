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