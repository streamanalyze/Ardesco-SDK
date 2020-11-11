/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 * 
 * coproc_common has defines for calling the cpu specific
 * low level code for coprocessor communication. Apps
 * shouldn't need to include this file.
 */
#ifndef ARD_COPROC_COM_H__
#define ARD_COPROC_COM_H__

int ipc_lowlevel_sendstring(char *str);

typedef int (*coproc_recv_cb)(char *cmd, size_t len);

int ipc_lowlevel_init (coproc_recv_cb cb);

int ipc_lowlevel_shutdown (void);

void ipc_lowlevel_console_out (char *str);

#ifdef __cplusplus
}
#endif

#endif //ARD_COPROC_COM_H__