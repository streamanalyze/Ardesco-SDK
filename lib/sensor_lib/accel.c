/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>
#include <stdio.h>
#include <string.h>
#include <logging/log.h>
#include <drivers/sensor.h>
#include "accel_sensor.h"

#define ACCELLIB_VERSION_MAJOR      0
#define ACCELLIB_VERSION_MINOR      1

// Accel driver name
#define ACCEL_MU            "m/s^2"
#define ACCEL_FAULT         0.03f
#define ACCEL_ACCURACY      (float)((1 - ACCEL_FAULT) * 100)

#define ACCEL_ATTRIBUTES    {               \
            SENSOR_ATTR_FULL_SCALE,         \
            SENSOR_ATTR_SAMPLING_FREQUENCY, \
            SENSOR_ATTR_LOWER_THRESH,       \
            SENSOR_ATTR_UPPER_THRESH        \
        }

#define ACCEL_TRIGGERS  {           \
            SENSOR_TRIG_THRESHOLD,  \
            SENSOR_TRIG_DATA_READY  \
        }

#define DEFAULT_TRIGGER     SENSOR_TRIG_THRESHOLD

//Accel device has binded these channels
#define ACCEL_CHANNELS       {      \
            SENSOR_CHAN_ACCEL_X,    \
            SENSOR_CHAN_ACCEL_Y,    \
            SENSOR_CHAN_ACCEL_Z     \
        }

int accl_channels[] = ACCEL_CHANNELS;
struct sensor_value accl_raw_data[ARRAY_SIZE(accl_channels)];

LOG_MODULE_REGISTER(accel, CONFIG_APP_LOG_LEVEL);

static void handle_data_callback(void *in, void *out, uint32_t *psize);

/**@brief Callback for sensor trigger events */
static void sensor_trigger_handler(uint32_t reason, void *data, int len, uint32_t userdata)
{
    //printk ("Accel trigger exec.\r\n");
    return;
}

//====================================================
// ardaccel_init - Initializes the library
//====================================================
void *ardaccel_init(int *prc, char *driver_name)
{
    int rc = -1;
    struct senlib *lib = NULL;
    //char device_name[] = ACCEL_DEV_NAME;

    struct senlib_sensor sensor = {
        .dev_name = driver_name,
        .channels = accl_channels,
        .no_of_channels = ARRAY_SIZE(accl_channels),
        .raw_data = accl_raw_data
    };

    struct sensor_trigger sensor_trig = {
        .type = SENSOR_TRIG_THRESHOLD,
    };
    
    lib = senlib_init(&sensor, handle_data_callback, prc);
    if (lib == 0)
    {
        LOG_ERR("Unable to initialize accelerometer");
        return 0;
    }

    rc = senlib_settrigger (lib, &sensor_trig, sensor_trigger_handler, 0);
    if (rc) {
        LOG_ERR("Unable to set accelerometer trigger");
        *prc = rc;
        return 0;
    }

    return lib;
}	

//====================================================
// ardaccel_deinit - Deinitializes the library
//====================================================
int ardaccel_deinit(void *h)
{
    struct senlib *lib = (struct senlib *)h;
    
    if (lib == 0)
    {
            LOG_ERR("Invalid handle in %s\n", __FUNCTION__);
            return EINVAL;
    }
    senlib_deinit(lib);

    return 0;
}

//====================================================
// ardaccel_read - Read data to the server.
//====================================================
int ardaccel_read (void *h, void *pData, int nSize)
{
    int rc = 0;
    int data_size = 0;
    struct senlib *lib = (struct senlib *)h;

    if ((lib == 0) || (pData == 0))
	{
		LOG_ERR("Invalid handle in %s\n", __FUNCTION__);
		return -EINVAL;
	}

    if (nSize < sizeof(accel_data_t)) 
    {
		LOG_ERR("buffer to small in %s\n", __FUNCTION__);
		return -EINVAL;
    }
    // declare typed pointer to output buffer.
    data_size = senlib_readsensor(h, pData, nSize);
    if (data_size != nSize) {
        rc = -EINVAL;
    }
    return rc;
}

//====================================================
// ardaccel_configure - Configure the library.
//====================================================
int ardaccel_configure(void *h, unsigned int Func, void *pData, uint32_t *pnSize)
{
    int rc = 0;
    struct senlib *lib = (struct senlib *)h;
    struct accelsetcbstruct *cbs = (struct accelsetcbstruct *)pData;
    float *accuracy = pData;
    const char m_units[] = ACCEL_MU;
	uint8_t no_of_munits = sizeof(m_units) / sizeof(m_units[0]);
    enum accel_limits {
        ACCEL_LOW_THRESH,
        ACCEL_HIGH_THRESH,
        ACCEL_THRESH_COUNT
    };

    // enum sensor_attribute accel_limits_attr[ACCEL_THRESH_COUNT] = {
    //     [ACCEL_LOW_THRESH] = SENSOR_ATTR_LOWER_THRESH,
    //     [ACCEL_HIGH_THRESH] = SENSOR_ATTR_UPPER_THRESH,
    // };

    if ((Func != ARDCONFIG_GETVERSION) && (lib == 0))
	{
		LOG_ERR("Invalid handle in %s\n", __FUNCTION__);
		return EINVAL;
	}

    switch (Func)
    {
        case ARDCONFIG_GETVERSION:
            if ((pnSize == 0) || (*pnSize <= sizeof(uint32_t)) || (pData == 0))
            {
                LOG_ERR("Invalid parameter in %s\n", __FUNCTION__);
                return EINVAL;
            }
            // Write the version of the library
            *((uint16_t *)pData) = ACCELLIB_VERSION_MAJOR;
            *((uint16_t *)pData+1) = ACCELLIB_VERSION_MINOR;
            break;
        case ARDCONFIG_SETCALLBACK:
            if ((pnSize == 0) || (*pnSize != sizeof(struct accelsetcbstruct)) || (pData == 0))
            {
                LOG_ERR("Invalid parameter in %s\n", __FUNCTION__);
                return EINVAL;
            }
            senlib_savecb(lib, cbs->fn, cbs->userdata);
            break;
        // case ARDCONFIG_SETMSRTIMER:
        //     SensorLib_SetTimer(lib, *(s32_t *)pData);
        //     break;
        case ARDCONFIG_GETUNITS:
            for(uint8_t idx = 0; idx < no_of_munits; idx++)
            {
                *pnSize = strlen(&m_units[idx]) + 1;
                memcpy((char *)pData, m_units, *pnSize);
            }
            break;
        case ARDCONFIG_GETACCURACY:
            *accuracy = ACCEL_ACCURACY;
            break;
        case ARDCONFIG_GETFREQUENCY:
            //TODO
            break;
        // case ARDCONFIG_SETLIMIT:
        //     if(!does_sensor_contain_attr(accel_limits_attr[ACCEL_LOW_THRESH]) ||
        //        !does_sensor_contain_attr(accel_limits_attr[ACCEL_HIGH_THRESH])  ) {
        //         return EINVAL;
        //     }
        //     rc = SensorLib_SetAttributes(lib->dev, accel_limits_attr, 
        //                 (int *)pData, ACCEL_THRESH_COUNT);
        default:
            LOG_ERR("Invalid Function in %s\n", __FUNCTION__);
            rc = EINVAL;
            break;
    }

    return rc;
}
//----------------------------------------------------
// handle_data_callback - receives callback from common code.
//----------------------------------------------------
static void handle_data_callback(void *in, void *out, uint32_t *psize)
{
    struct sensor_value *in_data = in;
    accel_data_t *out_data = out;
    enum accel_channels_order {
        ACCEL_CHAN_X,
        ACCEL_CHAN_Y,
        ACCEL_CHAN_Z
    };

    out_data->x = sensor_value_to_double(&in_data[ACCEL_CHAN_X]);
    out_data->y = sensor_value_to_double(&in_data[ACCEL_CHAN_Y]);
    out_data->z = sensor_value_to_double(&in_data[ACCEL_CHAN_Z]);
    *psize = sizeof(accel_data_t);
}
