/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#ifndef ARD_ARDESCO_H__
#define ARD_ARDESCO_H__

// Include basic operating system dependencies
#include <zephyr.h>
#include <sys/types.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _COUNTOF(a) (sizeof(a)/sizeof(a[0]))


#ifdef CONFIG_BOARD_NRF9160_ARD0011ANS
#endif //CONFIG_BOARD_NRF9160_ARD0011ANS

// Ardesco Combi and Combi Dev
#if defined CONFIG_BOARD_NRF9160_ARD0021BNS || defined CONFIG_BOARD_NRF9160_ARD0022BNS
#endif //CONFIG_BOARD_NRF9160_ARD0021BNS || CONFIG_BOARD_NRF9160_ARD0022BNS
#if defined CONFIG_BOARD_NRF52840_ARD0021BNS || defined CONFIG_BOARD_NRF52840_ARD0022BNS
#endif //CONFIG_BOARD_NRF52840_ARD0021BNS || CONFIG_BOARD_NRF52840_ARD0022BNS

#ifdef CONFIG_BOARD_NRF9160_ARD0031ANS
#endif //CONFIG_BOARD_NRF9160_ARD0031ANS


//
// The memory allcators/frees are dereferened here.
// 1. Simple replacement if needed.
// 2. Avoid use of stdlib if only for simple memory ops
// 3. To centralize the CONFIG_HEAP_MEM_POOL_SIZE error.
//

// If the error "undefined reference to `k_malloc'" is shown, 
// add CONFIG_HEAP_MEM_POOL_SIZE=xxx to prj.conf where xxx is a number 
// such as 2028.
static inline void * ard_malloc (size_t size) {
	return k_malloc (size);
}

// Free dereference
static inline void ard_free(void *mem) {
	return k_free (mem);
}


//
// Library related values
//

// Standard config functions are listed below
#define ARDCONFIG_GETVERSION		0
#define ARDCONFIG_SETCALLBACK		1
#define ARDCONFIG_SETMSRTIMER       2
#define ARDCONFIG_GETUNITS          3
#define ARDCONFIG_GETACCURACY       4
#define ARDCONFIG_GETFREQUENCY      5
#define ARDCONFIG_SETLIMIT          6
#define ARDCONFIG_GETLIMIT          7

// Library specific config functions start at the value below
#define ARDCONFIG_LIBSPECIFIC  0080

/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif /* ARD_ARDESCO_H__ */
