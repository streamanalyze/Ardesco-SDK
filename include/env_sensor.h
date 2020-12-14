/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */
#ifndef ARD_ENV_H_
#define ARD_ENV_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <zephyr/types.h>
#include "sensor_common.h"

// Structure to pass env data
typedef struct {
	double temperature;	
	double humidity;	
	double pressure;	
} env_data_t;


void *ardenv_init(int *prc);
int ardenv_deinit(void *h);
int ardenv_read (void *h, void *pData, int nSize);
int ardenv_configure(void *h, unsigned int Func, void *pData, uint32_t *pnSize);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ARD_ENV_H_ */
