/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>
#include <stdio.h>
#include <string.h>
#include <logging/log.h>
#include <drivers/sensor.h>
#include "env_sensor.h"

#define ENVLIB_VERSION_MAJOR      0
#define ENVLIB_VERSION_MINOR      1

// Sensor driver name
#define ENV_DEV_NAME    "BME680"
#define ENV_MU	{	\
            "C", 	\
            "g/mc",	\
            "hPa",	\
        }
#define ENV_FAULT         0.05f
#define ENV_ACCURACY      (float)((1 - ENV_FAULT) * 100)

LOG_MODULE_REGISTER(env, CONFIG_APP_LOG_LEVEL);

static void handle_data_callback(void *in, void *out, uint32_t *psize);

//====================================================
//====================================================


//ENV device has binded these channels
#define ENV_CHANNELS       {            \
            SENSOR_CHAN_AMBIENT_TEMP,   \
            SENSOR_CHAN_HUMIDITY,    	\
            SENSOR_CHAN_PRESS     	\
        }

int env_channels[] = ENV_CHANNELS;
struct sensor_value env_raw_data[ARRAY_SIZE(env_channels)];

//====================================================
// ardenv_init - Initializes the library
//====================================================
void *ardenv_init(int *prc)
{
    struct senlib *lib = NULL;
    char device_name[] = ENV_DEV_NAME;

    struct senlib_sensor sensor = {
        .dev_name = device_name,
        .channels = env_channels,
        .no_of_channels = ARRAY_SIZE(env_channels),
        .raw_data = env_raw_data
    };
    
    lib = senlib_init(&sensor, handle_data_callback, prc);
    
    return lib;
}	

//====================================================
// ardenv_deinit - Deinitializes the library
//====================================================
int ardenv_deinit(void *h)
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
// ardenv_read - Read data to the server.
//====================================================
int ardenv_read (void *h, void *pData, int nSize)
{
    int rc = 0;
    int data_size = 0;
    struct senlib *lib = (struct senlib *)h;

    if ((lib == 0) || (pData == 0))
	{
		LOG_ERR("Invalid handle in %s\n", __FUNCTION__);
		return -EINVAL;
	}

    if (nSize < sizeof(env_data_t)) 
    {
		LOG_ERR("buffer to small in %s\n", __FUNCTION__);
		printk ("buffer to small in %s\r\n", __FUNCTION__);
		return -EINVAL;
    }
    // declare typed pointer to output buffer.
    data_size = senlib_readsensor(h, pData, nSize);
    if (data_size != nSize) {
        printk ("Invalid data size %d. Expecting %d\r\n", nSize, data_size);
        rc = -EINVAL;
    }
    return rc;
}

//====================================================
// ardenv_configure - Configure the library.
//====================================================
int ardenv_configure(void *h, unsigned int Func, void *pData, uint32_t *pnSize)
{
    int rc = 0;
    struct senlib *lib = (struct senlib *)h;
    struct envsetcbstruct *cbs = (struct envsetcbstruct *)pData;
    float *accuracy = pData;
    const char *m_units[] = ENV_MU;
    uint8_t no_of_channels = sizeof(m_units) / sizeof(m_units[0]);

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
            *((uint16_t *)pData) = ENVLIB_VERSION_MAJOR;
            *((uint16_t *)pData+1) = ENVLIB_VERSION_MINOR;
            break;
        case ARDCONFIG_SETCALLBACK:
            if ((pnSize == 0) || (*pnSize != sizeof(struct envsetcbstruct)) || (pData == 0))
            {
                LOG_ERR("Invalid parameter in %s\n", __FUNCTION__);
                return EINVAL;
            }
            senlib_savecb (lib, cbs->fn, cbs->userdata);
            break;
        // case ARDCONFIG_SETMSRTIMER:
        //     SensorLib_SetTimer(lib->dev, *(s32_t *)pData);
        //     break;
        case ARDCONFIG_GETUNITS:
            for(uint8_t idx = 0; idx < no_of_channels; idx++)
            {
                *pnSize = strlen(m_units[idx]) + 1;
                memcpy((char *)pData, m_units, *pnSize);
            }
            break;
        case ARDCONFIG_GETACCURACY:
            *accuracy = ENV_ACCURACY;
            break;
        case ARDCONFIG_GETFREQUENCY:
            //TODO
        case ARDCONFIG_SETLIMIT:
            //TODO
        default:
            LOG_ERR("Invalid Function in %s\n", __FUNCTION__);
            rc = EINVAL;
            break;
    }

    return rc;
}
//----------------------------------------------------
// handle_data_callback - Handles callback from common code.
//----------------------------------------------------
static void handle_data_callback(void *in, void *out, uint32_t *psize)
{
    struct sensor_value *in_data = in;
    env_data_t *out_data = out;
    enum accel_channels_order {
        TEMPERATURE,
        HUMIDITY,
        PRESSURE
    };
    out_data->temperature = sensor_value_to_double(&in_data[TEMPERATURE]);
    out_data->humidity = sensor_value_to_double(&in_data[HUMIDITY]);
    out_data->pressure = sensor_value_to_double(&in_data[PRESSURE]);
    *psize = sizeof(env_data_t);
}
