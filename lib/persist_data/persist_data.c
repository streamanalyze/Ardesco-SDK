/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>
#include <stdlib.h>
#include <stdio.h>
#include <settings/settings.h>
#include "persist_data.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(app_persist_data, CONFIG_APP_LOG_LEVEL);

#define IS_PERSIST_STRING(a) ((a->flags & PERSIST_DATA_TYPE_MASK) == PERSIST_DATA_TYPE_STRING)
#define IS_PERSIST_INT(a)    ((a->flags & PERSIST_DATA_TYPE_MASK) == PERSIST_DATA_TYPE_INT)

static persist_data_t *s_datas;
static int s_data_count;

persist_data_t* persist_data_find(const char* name);

//
// set - Sets the data into the persistent store
//
static int set(const char *key, size_t len_rd, settings_read_cb read_cb,
	       void *cb_arg)
{
	int len;

	if (key == NULL) {
		return -ENOENT;
	}

	persist_data_t *param = persist_data_find(key);
	if (param == NULL) {
		LOG_ERR("Unable to find persist param %s", log_strdup(key));
		return -ENOENT;
	}

	len = read_cb(cb_arg, param->data, param->size);
	if (len < param->size) {
		LOG_ERR("Unable to read persist param %s. Resetting.", log_strdup(key));
		memset(param->data, 0, param->size);
	}

	return 0;
}

static struct settings_handler handler = {
	.name = "params",
	.h_set = set,
};

int persist_data_init(persist_data_t *params, int param_count)
{
	int err = 0;

	// init persistent settings subsys
	err = settings_subsys_init();
	if (err) {
		printk("settings_subsys_init failed (%d)", err);
		return err;
	}

	s_datas = params;
	s_data_count = param_count;

	err = settings_register(&handler);
	if (err) {
		LOG_ERR("settings_register failed (err %d)", err);
		return err;
	}

	return 0;
}

// int persist_data_get_string(persist_data_t *param, const char** val)
// {
// 	if (IS_PERSIST_STRING(param)) {
// 		*val = param->data;
// 		return 0;
// 	}
// 	return -EINVAL;
// }

// int persist_data_get_int(persist_data_t *param, int* val)
// {
// 	if (IS_PERSIST_INT(param)) {
// 		*val = *((int*)param->data);
// 		return 0;
// 	}
// 	return -EINVAL;
// }

// int persist_data_get_as_string(persist_data_t *param, char* buf, size_t size)
// {
// 	int would_write = -1;
// 	if (IS_PERSIST_STRING(param)) {
// 		would_write = snprintf(buf, size, "%s", (char*)param->data);
// 	} else 	if (IS_PERSIST_INT(param)) {
// 		would_write = snprintf(buf, size, "%d", *((int*)param->data));
// 	}
// 	int err = ((0 <= would_write) && (would_write < size)) ? 0 : -EINVAL;
// 	return err;
// }

static int do_save(persist_data_t *param)
{
	char buf[24];
	snprintf(buf, sizeof(buf), "params/%s", param->name);
	int err =  settings_save_one(buf, param->data, param->size);
	return err;
}

// int persist_data_set_string(const char *name, const char* val)
// {
// 	int err = -EINVAL;
// 	if (strlen(val) < param->size) {
// 		strcpy(param->data, val);
// 		err = do_save(param);
// 	}
// 	return err;
// }

persist_data_t* persist_data_find(const char* name)
{
	for (int i = 0; i < s_data_count; i++) {
		if (strcmp(name, s_datas[i].name) == 0) {
			return &s_datas[i];
		}
	}
	return NULL;
}

// int persist_data_set_int_i(persist_data_t *param, int val)
// {
// 	*((int*)param->data) = val;
// 	return do_save(param);
// }
int persist_data_set_int(const char *name, const int val)
{
	int err = -EINVAL;

	persist_data_t *param = persist_data_find(name);
	if (param == NULL) {
		LOG_ERR("Unable to find persist param %s", log_strdup(name));
		return -ENOENT;
	}

	if (IS_PERSIST_INT(param)) {
		*((int*)param->data) = val;
		err = do_save(param);
	} 

	return err;
}

int persist_data_set_string(const char *name, const char* val)
{
	int err = -EINVAL;

	persist_data_t *param = persist_data_find(name);
	if (param == NULL) {
		LOG_ERR("Unable to find persist param %s", log_strdup(name));
		return -ENOENT;
	}

	if (IS_PERSIST_STRING(param)) {
		if (strlen(val) < param->size - 1) {
			strcpy(param->data, val);
		}
	} else if (IS_PERSIST_INT(param)) {
		int v = atoi(val);
		if (param->size == sizeof(int)) {
			*((int*)param->data) = v;
		}
	} 
	if (err == 0) {
		err = do_save(param);
	}
	return err;
}

int persist_data_get_int (const char *name, int*out)
{
	int rc = 0;

	// Find the entry in the array
	persist_data_t *param = persist_data_find(name);
	if (param == NULL) {
		LOG_ERR("Unable to find persist param %s", log_strdup(name));
		return -ENOENT;
	}
	if (IS_PERSIST_INT(param)) {	// Copy pointer to data.
		*out = (int)param->data;
	} else
		rc = -EINVAL;
	return rc;
}

int persist_data_get_string_ptr (const char *name, const char **out)
{
	int rc = 0;

	// Find the entry in the array
	persist_data_t *param = persist_data_find(name);
	if (param == NULL) {
		LOG_ERR("Unable to find persist param %s", log_strdup(name));
		return -ENOENT;
	}
	
	// Copy pointer to data.
	if (IS_PERSIST_STRING(param)) {	// Copy pointer to data.
		*out = param->data;
	} else
		rc = -EINVAL;

	return rc;
}

int persist_data_get_string (const char *name, char *out, int outsize)
{
	int rc = 0;

	// Find the entry in the array
	persist_data_t *param = persist_data_find(name);
	if (param == NULL) {
		LOG_ERR("Unable to find persist param %s", log_strdup(name));
		return -ENOENT;
	}
	
	// Copy string data.
	if (IS_PERSIST_STRING(param)) {	
		if (param->size < outsize)
			strcpy (out, param->data);
		else
			rc = -ENOSR;
	}
	else
		rc = -EINVAL;
	return rc;
}