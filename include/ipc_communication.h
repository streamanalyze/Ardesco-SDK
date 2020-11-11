/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#ifndef ARD_COPROC_COMMUNICATION_H__
#define ARD_COPROC_COMMUNICATION_H__

#ifdef __cplusplus
extern "C" {
#endif

// ipc common communication strings
#define ATTN_STR  "+AT"
#define EOL_STR   "\n"
#define EOL_CHR   '\n'

#define OK_STR    "OK"
#define ERR_STR   "ERR"

// Command string length is limited by usb_uart buffer size 
#define MAX_CMD_LEN  (40 - _COUNTOF(ATTN_STR) - 1)

// Common commands across all IPC implemenations
#define CMD_STR_DBGOUT  "DBOUT"
#define CMD_STR_ECHO    "ECHO"
#define CMD_STR_GETVER  "GETVER"

// Currently supported error codes.
#define BADCMD_CODE  4
#define BADCMD_TXT   "BADCMD"

#define BUSY_CODE  7
#define BUSY_TXT   "BUSY"


// Command dispath function template
typedef int (*cmd_fn)(char *cmd, char *args);

// Struct used for command table.
struct command_table {
	char *name;
	cmd_fn fn;
};

/*
 * Initialize coprocessor communication.
 */
int ipc_init ();

/*
 * Initialize coprocessor communication. Use this function to
 * provide a list of custom commands for the IPC library.
 */
// 
int ipc_init_extended (struct command_table *p, int command_cnt);

/*
 * Shut down the IPC comms
 */
int ipc_shutdown (void);

/*
 * Send a command to the coprocessor
 */
int ipc_cmd_send (char *cmd, bool fwait);

/*
 * Send a command to be echoed back to this application.
 */
int ipc_echo_cmd (char *cmd);

/*
 * Send a string out the console of the other proc
 */
int ipc_dbg_out (char *str);

/*
 * ipc_get_coproc_ver - Queries the version and compile
 * time of the other processor's firmware.
 */
int ipc_get_coproc_ver (char *resp, int siz);

#ifdef __cplusplus
}
#endif

#endif //ARD_COPROC_COMMUNICATION_H__