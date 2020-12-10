/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */
#ifndef SENSOR_COMMON_H_
#define SENSOR_COMMON_H_

#include <zephyr/types.h>
#include <drivers/sensor.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Flags set when a trigger ocurrs */
#define REASONNUMBER(a)            (1 << a)
#define ARDCB_EN_ALL                  0xFFFFFFFF
#define ARDCB_EN_LIBERROR             0x0001
#define ARDCB_EN_DATAREADY            0x0002
#define ARDCB_EN_LIMITEXCEEDED        0x0003

/* Data could not be obtained from the device */
#define ARDCB_LIBERROR         REASONNUMBER(ARDCB_EN_LIBERROR)
/* Data was read from the device*/
#define ARDCB_DATAREADY        REASONNUMBER(ARDCB_EN_DATAREADY)
/* Data was obtained from the device when a trigger ocurred*/
#define ARDCB_LIMITEXCEEDED    REASONNUMBER(ARDCB_EN_LIMITEXCEEDED)


// Add an id field for debugging
#define USE_ID_TAG 
#ifdef USE_ID_TAG
#define SENSTRUCT_TAG 0xdeadbeef
#endif

/**
 * @brief Callback function called after data is read to format 
 * that data
 *
 * @param read_data Pointer to data which was read from the device.
 * Usually is of type array of sensor_value with size as the number
 * of channels
 * @param out_data Pointer to the formatted data 
 * @param psize Size of the formatted data
 */
typedef void (*SenLib_data_handler_cb)(void *read_data, void *out_data, uint32_t *psize);

/**
 * @brief Callback function called after a trigger ocurrs
 *
 * @param reason Value of type ARDCB_xxx
 * @param data Pointer to data read from the device. May be formatted with 
 * SenLib_data_handler_cb before.
 * @param len Size of data which was read and formatted
 * @param userdata Additional data specific to device
 */
typedef void (*SenLib_trigger_fn)(uint32_t reason, void *data, int len, uint32_t userdata);


/**
 * Sensor device definition according to the SensorLib. 
 * Used to configure a device at the Init step.
 */
struct senlib_sensor {
    /* Device string used to obtain device pointer */
    char *dev_name;
    /* Trigger used for devices which have driver enabled
     * trigger operation
     */
    enum sensor_trigger_type trigger;
    /* Pointer to all of a device channels ids
     * in the format of enum sensor_channel
     */
    int *channels;
    /* Total number of channels of a device */
    uint8_t no_of_channels;

    //(db) Added
    struct sensor_value *raw_data;
};

// Setcallback structure
struct envsetcbstruct {
    uint32_t reasonflags;
    SenLib_trigger_fn fn;
    uint32_t userdata;
};

/*
 * 
 */
void *senlib_init (struct senlib_sensor *in_sensor, 
                   SenLib_data_handler_cb cb, int *prc);

/*
 * 
 */
void senlib_deinit (void *lib_in);

/*
 * 
 */
int senlib_readsensor(void *lib_in, void *out_data, int size);

/*
 * 
 */
int senlib_savecb (void *lib_in, SenLib_trigger_fn fn, uint32_t userdata);

/*
 * 
 */
int senlib_settrigger (void *lib_in, struct sensor_trigger *trig, SenLib_trigger_fn fn, uint32_t userdata);


#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* SENSOR_COMMON_H_ */
