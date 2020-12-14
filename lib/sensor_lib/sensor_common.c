/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>
#include <stdio.h>
#include <string.h>

#include "sensor_common.h"

/*
 * senlib_struct - Structure used to support sensor instance data.
 * The strucure is defined here as it is private to is file.
 */ 
struct senlib_struct {
    // These first structures must be kept in order as the code
    // depends on their placement and size    
#ifdef USE_ID_TAG
    uint32_t idtag;
#endif
    struct k_work trig_work;        // Driver trigger work structure*/
    struct sensor_trigger trig;     // Trigger structure
    struct k_work timer_work;       // Timer period trigger work structure*/
    struct k_timer trigger_timer;   // Timer associated with a timer*/

    bool use_periodic_measurement;  // Used to check if the timer can be started and stopped
    bool use_driver_trigger;        // Used to check if the driver trigger can be set or disabled

    struct senlib_struct *pnext;
    int Flags;

#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
    struct device *dev;             //  Pointer to the device driver 
#else
    const struct device *dev;       //  Pointer to the device driver 
#endif

    SenLib_data_handler_cb dhcb_fn;
    /* Callback function called when trigger ocurrs */
    SenLib_trigger_fn fn;
    /* User data*/
    uint32_t userdata;
    struct senlib_sensor sensor;
};

// start of the list of allocated sensor structures.
struct senlib_struct *listend = 0;

#define  TRIGGER_STRUCT          1
#define  TRIGGER_WORK_STRUCT     2
#define  TIMER_WORK_STRUCT       3
#define  TIMER_STRUCT            4
/*
 * get_lib_struct - locates the senlib structure based on either a pointer
 * to an internal structure within the base structure or by sequencing through
 * the list of senlib structures to find the matching device structure pointer.
 */  
#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
struct senlib_struct *get_lib_struct (struct device *dev, void *ptr, int type)
#else
struct senlib_struct *get_lib_struct (const struct device *dev, void *ptr, int type)
#endif
{
    //printk ("get_lib_struct dev %x  ptr %x  type %d\r\n", (uint32_t)dev, (uint32_t)ptr, type);
    uint32_t hexval = (uint32_t)ptr;
#ifdef USE_ID_TAG
    // back out structure tag
    hexval -= sizeof (uint32_t);
#endif
    do {
        if (type == TRIGGER_WORK_STRUCT)
            break;
        
        // back out trig_work struct 
        hexval -= sizeof (struct k_work);
        if (type == TRIGGER_STRUCT)
        {
            // we need a list lookup b/c trigger struct is copied to device
            struct senlib_struct *next = listend;
            while (next != 0)
            {
                if (next->dev == dev)
                {
                    hexval = (uint32_t)next;
                    break;
                }
                next = next->pnext;
            }
            break;
        }

        // back out sensor_trigger struct 
        hexval -= sizeof (struct sensor_trigger);
        if (type == TIMER_WORK_STRUCT)
            break;

        // back out trig_work struct 
        hexval -= sizeof (struct k_work);
        if (type == TIMER_STRUCT)
            break;

        printk ("Bad pointer type %d passed to get_lib_struct\r\n", type);
        return 0;
    } while (false);

    struct senlib_struct *lib = (struct senlib_struct *)hexval;
#ifdef USE_ID_TAG
    if (lib->idtag != SENSTRUCT_TAG)
    {
        printf ("bad pointer lookup!!!\n");
        return 0;
    }
#endif
    // See if we match the dev
    if ((dev != 0) && (lib->dev != dev))
    {
        printf ("bad pointer lookup 2!!!\n");
        return 0;
    }
    return lib;
}

/*
 *
 */
static void sensor_trigger_work_handler(struct k_work *work)
{
    struct senlib_struct *lib = get_lib_struct (0, work, TRIGGER_WORK_STRUCT);

    if (lib->fn)
    {
        // Call upper layer.
        (lib->fn)(ARDCB_LIMITEXCEEDED, (void *)&lib->trig, sizeof (struct sensor_trigger), lib->userdata);
    }    
    return;
}

/*
 *
 */
#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
static void sensor_trigger_handler(struct device *dev, struct sensor_trigger *trigger)
#else
static void sensor_trigger_handler(const struct device *dev, struct sensor_trigger *trigger)
#endif
{
	ARG_UNUSED(dev);
	ARG_UNUSED(trigger);

    struct senlib_struct *lib = get_lib_struct (dev, trigger, TRIGGER_STRUCT);

	k_work_submit(&lib->trig_work);
}

/*
 * senlib_settrigger
 */
int senlib_settrigger (void *lib_in, struct sensor_trigger *trig, SenLib_trigger_fn fn, uint32_t userdata)
{
    int rc;
    struct senlib_struct *lib = (struct senlib_struct *)lib_in;
    // Copy the trigger structure
    memcpy (&lib->trig, trig, sizeof (struct sensor_trigger));
    lib->fn = fn;
    lib->userdata = userdata;

    rc = sensor_trigger_set(lib->dev, &lib->trig, sensor_trigger_handler);

    return 0;
}
// Unused at this time.
static void sensor_timer_handler(struct k_work *work)
{
    return;
}


/*
 * senlib_init - Initialize the common sensor code for the sensor.
 */
void *senlib_init (struct senlib_sensor *in_sensor, SenLib_data_handler_cb cb,
                        int *prc)
{

    // Open the underlying device driver
#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
    struct device *dev = device_get_binding(in_sensor->dev_name);
#else
    const struct device *dev = device_get_binding(in_sensor->dev_name);
#endif
    if (dev == NULL) 
    {
        printk("Could not get %s device", in_sensor->dev_name);
        *prc = -ENODEV;
        return 0;
    }
    // If we can open the driver, alloc the structure we'll use.
    struct senlib_struct *lib = ard_malloc (sizeof (struct senlib_struct) + 
                                            (in_sensor->no_of_channels * sizeof (struct sensor_value)));
#ifdef USE_ID_TAG
    // Set tag
    lib->idtag = SENSTRUCT_TAG;
#endif

    // Link the alloced structure so we can do a reverse lookup.
    lib->pnext = listend;
    listend = lib;

    // Copy the data provided by the upper layer.
    memcpy (&lib->sensor, in_sensor, sizeof (struct senlib_sensor));

    // Copy device handle
    lib->dev = dev;

    // Init the callback and timer work structures.
    k_work_init(&lib->trig_work, sensor_trigger_work_handler);
    k_work_init(&lib->timer_work, sensor_timer_handler);
    lib->dhcb_fn = cb;

    return lib;
}
/*
 * senlib_deinit - Free up sensor resources.
 */
void senlib_deinit (void *lib_in)
{
    struct senlib_struct *lib = (struct senlib_struct *)lib_in;

    // we need to find the structure in the list and delink it
    struct senlib_struct *next = listend;
    if (lib == next)
    {
        listend = lib->pnext;
    }
    else
    {
        struct senlib_struct *old = listend;
        while (next != 0)
        {
            if (lib == next)
            {
                old->pnext = lib->pnext;
                break;
            }
            old = next;
            next = next->pnext;
        }
    }
    ard_free (lib);
    return;
}

/*
 * senlib_readsensor - Read the sensor
 */
int senlib_readsensor(void *lib_in, void *out_data, int size)   
{
    struct senlib_struct *lib = (struct senlib_struct *)lib_in;

    double *data = (double *)out_data;
    
    if (size < (lib->sensor.no_of_channels * sizeof (double)))
    {
        return -ENOSR;
    }

	int err;

    err = sensor_sample_fetch_chan(lib->dev, SENSOR_CHAN_ALL);
    if (err) {
        printk("Failed to fetch data from %s, error: %d\n",
            lib->sensor.dev_name, err);

        return err;
    }
    int data_size = 0;
	for (int i = 0; i < lib->sensor.no_of_channels; i++) 
    {
		err = sensor_channel_get(lib->dev, lib->sensor.channels[i], &lib->sensor.raw_data[i]);
		if (err) {
			printk("Failed to fetch data from %s, error: %d\n",
				lib->sensor.dev_name, err);
		} else {
			//(db)k_spinlock_key_t key = k_spin_lock(&(env_sensors[i]->lock));

			data[i] = sensor_value_to_double(&lib->sensor.raw_data[i]);
			//(db)k_spin_unlock(&(env_sensors[i]->lock), key);
            data_size += sizeof (double);
		}
	}
    return data_size;
}

/*
 * senlib_savecb - Save callback function pointer
 */
int senlib_savecb (void *lib_in, SenLib_trigger_fn fn, uint32_t userdata)
{
    struct senlib_struct *lib = (struct senlib_struct *)lib_in;
    lib->fn = fn;
    lib->userdata = userdata;
    return 0;
}
