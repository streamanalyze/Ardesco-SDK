/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#ifndef PERSIST_DATA_H__
#define PERSIST_DATA_H__

#include <ardesco.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Persistent data types 
#define 	PERSIST_DATA_TYPE_HUGE  	0x8000
#define 	PERSIST_DATA_TYPE_STRING 	0x4000
#define 	PERSIST_DATA_TYPE_INT 		0x2000
#define 	PERSIST_DATA_TYPE_MASK		0xF000

// Persistent data flags
#define 	PERSIST_DATA_ACCESS_READ 	0x0800
#define 	PERSIST_DATA_ACCESS_WRITE 	0x0400


//struct persist_data_s;
typedef struct persist_data_s persist_data_t;

/**
 * A persistent parameter
 */
struct persist_data_s {
	const char *name;
	u16_t flags;
	u16_t size;
	void *data;
	//persist_data_validate_cb_t validate_cb;
};

//
// Register persistent data array.
//

/**
 * Initiates a list of persist parameters
 *
 * @param params a pointer to the param list
 * @param param_count parameter count
 *
 * @return 0 if the operation was successful, otherwise a (negative) error code.
 */
int persist_data_init(persist_data_t *params, int param_count);

/**
 * Gets a persist string param
 *
 * @param param a pointer to the param
 * @param val a pointer to the value
 *
 * @return 0 if the operation was successful, otherwise a (negative) error code.
 */
int persist_data_get_string_ptr(const char *name, const char** val);

int persist_data_get_string (const char *name, char *out, int outsize);

/**
 * Gets a persist int param
 *
 * @param param a pointer to the param
 * @param val a pointer to the value
 *
 * @return 0 if the operation was successful, otherwise a (negative) error code.
 */
//int persist_data_get_int(persist_data_t *param, int* val);
int persist_data_get_int (const char *name, int*out);

/**
 * Gets a persist param as a string
 * The value is copied to given buffer
 *
 * @param param a pointer to the param
 * @param buf a pointer to the buffer
 * @param size the buffer size
 *
 * @return 0 if the operation was successful, otherwise a (negative) error code.
 */
int persist_data_get_as_string(persist_data_t *param, char* buf, size_t size);

/**
 * Sets a string persist param
 *
 * @param param a pointer to the param
 * @param val the value
 *
 * @return 0 if the operation was successful, otherwise a (negative) error code.
 */
int persist_data_set_string(const char *name, const char* val);

/**
 * Sets a integer persist param
 *
 * @param param a pointer to the param
 * @param val the value
 *
 * @return 0 if the operation was successful, otherwise a (negative) error code.
 */
int persist_data_set_int(const char *name, int val);

/**
 * Sets a persist param as string
 *
 * @param param a pointer to the param
 * @param val the value
 *
 * @return 0 if the operation was successful, otherwise a (negative) error code.
 */
int persist_data_set_as_string(const char *name, const char* val);

/**
 * Finds a persist param by name
 *
 * @param name the param name
 *
 * @return a pointer to a param if the operation was successful, otherwise NULL.
 */
//persist_data_t* persist_data_find(const char* name);

/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif /* PERSIST_DATA_H__ */
