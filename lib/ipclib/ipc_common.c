/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 * 
 * coproc_common - common code used by both the 
 * 9160 and 52840 for commuication between the two chips.
 * 
 */

#include <ardesco.h>
#include <stdio.h>
#include <string.h>

#include <ipc_communication.h>
#include <ipc_lowlevel.h>

#define ERR_RESP_TEMPLATE ERR_STR" %d %s"

void sendresponse (char *resp);
void senderrorresponse (int code, char *resp);
char *nextchar (char *str, bool space);

int nBusy = 0;

int commands_in_process = 0;
int last_error = 0;

// Ptr to command table
struct command_table *cmdtblptr;
int command_table_cnt = 0;

/*
 * debugoutcmd - prints string out the debug
 * console. Useful for when the USB is disconnected.
 * Example:
 *  DBOUT this string will print\n
 */
char hack_buf[64];
int debugoutcmd (char *cmd, char *args)
{
	//printk ("debugoutcmd++ >%s< >%s<\n", cmd, args);

	int len = strlen (args);
	if (len)
	{
		// Print it. We need to copy the string as it
		strcpy (hack_buf, args);
		if (args[len-1] != '\n')
			strcat (hack_buf, "\r\n");
		ipc_lowlevel_console_out(hack_buf);
	}
	return 0;
}

/*
 * echocmd - echos the command passed back to
 * the other CPU for testing.
 */ 
int echocmd (char *cmd, char *args)
{
	printk ("echocmd cmd received\n");
	// Unlike other commands, we'll send the
	// OK first, then perform the actual
	// command.
	sendresponse (OK_STR);

	k_sleep (K_MSEC(300));
	ipc_lowlevel_sendstring (ATTN_STR);
	char *p = nextchar(cmd, true);
	ipc_lowlevel_sendstring (p);
	if (p[strlen (p)-1] != EOL_CHR)
		ipc_lowlevel_sendstring (EOL_STR);
	
	return -1; // We've already sent the response.
}
/*
 * getcoprocversion - Returns the version and
 * build time for the other CPU,
 * for example: 1.23 Sep 17 2020 15:05:49
 */
int getcoprocversion (char *cmd, char *args)
{
	char sz[64];
	// compose version compile time
	snprintf(sz, sizeof(sz), "%s %s %s %s", CMD_STR_GETVER, 
	         STRINGIFY(APP_VERSION), __DATE__, __TIME__);
	sendresponse (sz);

	// Now send the OK string
	sendresponse (OK_STR);
	return 0;
}

// Commands common to both CPUs
struct command_table commoncmdtbl [] = 
{
	{CMD_STR_DBGOUT, debugoutcmd},
	{CMD_STR_ECHO, echocmd},
	{CMD_STR_GETVER, getcoprocversion},
};
char last_cmd[64] ="";
/*
 * cmd_dispatch - dispatches the commands using
 * the lookup table passed by app at init.
 */ 
int cmd_table_lookup(char *cmdstr, char *args, struct command_table *cmdtblptr, int cmdcnt, int *fnd)
{
	int rc = 0, i;
	for (i = 0; i < cmdcnt; i++)
	{
		// Compare name
		if (strncmp(cmdstr, cmdtblptr[i].name, strlen (cmdtblptr[i].name)-1) == 0)
		{
			//printk ("cmd %s found.\n",cmdtbl[i].name);
			rc = (cmdtblptr[i].fn)(cmdstr, args);
			*fnd = 1;
			break;
		}
	}
	return rc;
};

/*
 * cmd_dispatch - dispatches the commands using
 * the lookup table passed by app at init.
 */ 
void cmd_dispatch(char *cmdstr)
{
	int rc = 0;
	//printk ("cmd_dispatch++ >%s<\n", cmdstr);

	// if not empty command
	if (strlen (cmdstr) > 0)
	{
		int fnd = 0;
		// skip past cmd and any whitespace
		char *args = nextchar(cmdstr, true);
		args = nextchar(args, false);

		// Look up cmd in extended table.
		rc = cmd_table_lookup (cmdstr, args, cmdtblptr, command_table_cnt, &fnd);
		// If not found, see if a common command.
		if (!fnd)
		{
			rc = cmd_table_lookup (cmdstr, args, commoncmdtbl, _COUNTOF(commoncmdtbl), &fnd);
			if (!fnd)
			{
				printk ("bad cmd >%s<\n", cmdstr);
				rc = BADCMD_CODE;
			}
		}
	}
	// If rc negative, don't send any response.
	if (rc >= 0)
	{
		if (rc != 0)
		{
			//printk ("cmd >%s< error %d\n", cmdstr, rc);
			senderrorresponse (rc, "");
		}
		else
			sendresponse (OK_STR);
	}
}

/*
 * sendresponse - sends response string to 
 * coprocessor.
 */ 
void sendresponse (char *resp)
{
	ipc_lowlevel_sendstring (resp);
	ipc_lowlevel_sendstring (EOL_STR);
	if (nBusy)
		nBusy--;
}

/*
 * senderrorresponse - sends error response 
 * string to coprocessor.
 */ 
void senderrorresponse (int code, char *resp)
{
	char str[32];
	printk ("send err %d >%s<\n", code, resp);
	sprintf (str, ERR_RESP_TEMPLATE, code, resp);
	sendresponse (str);
}
// Trival scan for next char or space
char *nextchar (char *str, bool space)
{
	while ((*str != '\0'))
	{
		if (!space && (*str > ' '))
			break;
			
		if (space && (*str <= ' '))
			break;
			
		str++;
	}
	return str;
}

#ifdef DEBUG_CODE
void dumpbuff (char *title, char *p, int len)
{
	printk ("%s. len %d data >", title, len);
	for (int i = 0; i < len; i++)
	{
		if (p[i] == '\n') 
		{
			printk ("<NL>");
		}
		else if (p[i] == '\r') 
		{
			printk ("<CR>");
		}
		else if ((p[i] < ' ') || (p[i] > 'z'))
		{
			printk ("<%x>", p[i]);
		}
		else
			printk ("%c", p[i]);
	}
	printk ("<\n");
}
#endif
char resp_buff[64];
/*
* Callback from CPU specific serial code. Called on a 
* seperate thread that receives lines of text from a 
* fifo fed by an isr. The cmd buffer will be freed
* when this routine returns so if any async code
* must copy the contents of the command buffer.
*/
static int coproc_data_recved (char *cmd, size_t len)
{
#ifdef DEBUG_CODE
	int i;
	char buff[40];
	for (i = 0; i < _COUNTOF (buff)-1; i++) 
	{
		buff[i] = cmd[i];
		if ((cmd[i] == '\r') || (cmd[i] == '\n') || (cmd[i] == '\0'))
			break;
	}
	buff[i] = '\0';

	dumpbuff ("coproc msg", buff, len);
#endif

	// See if it's left over CRLF combos.
	char *p = nextchar (cmd, false);
	if (*p < ' ')
		return 0;
	
	//
	// check the header to see what we have.
	//
	if ((len >= _COUNTOF (ATTN_STR)-1) && 
	    (strncmp (cmd, ATTN_STR, _COUNTOF (ATTN_STR)-1) == 0))
	{
		// if not finished with last cmd, return busy.
		// if (nBusy > 0)
		// {
		// 	printk ("busy cnt %d\n", nBusy);
		// 	senderrorresponse (BUSY_CODE, BUSY_TXT);
		// 	return 0;
		// }
		nBusy++;
		//printk ("inc busy %d\n", nBusy);
		cmd_dispatch (nextchar (&cmd[_COUNTOF (ATTN_STR)-1], false));
		return 0;
	}
	int last_cmd_len = strlen (last_cmd);
	// See if it's the response to the last cmd.
	if ((len >= last_cmd_len) && 
	    (strncmp (cmd, last_cmd, last_cmd_len) == 0))
	{
		strncpy (resp_buff, cmd, sizeof (resp_buff));
		if (commands_in_process > 0)
			commands_in_process--;
	}
	// See if it's OK response.
	else if ((len >= _COUNTOF (OK_STR)-1) && 
	    (strncmp (cmd, OK_STR, _COUNTOF (OK_STR)-1) == 0))
	{
		//printk ("OK received\n");
		if (commands_in_process > 0)
			commands_in_process--;
	}

	// See if it's the response from the previous cmd.
	else if ((len >= _COUNTOF (ERR_STR)-1) && 
	    (strncmp (cmd, ERR_STR, _COUNTOF (ERR_STR)-1) == 0))
	{
		printk ("ERR received\n >%s<\n", cmd);
		if (commands_in_process > 0)
			commands_in_process--;
	}
  	else
	{
		printk ("Malformed packet >%s<\n", cmd);
	}
	return 0;
}

/*
 * ipc_cmd_send - Send a command to the coprocessor
 */
int send_nest = 0;
int ipc_cmd_send (char *cmd, bool fwait)
{
	int rc = -1;
 	//printk ("Sending: cmd >%s<\n", cmd);
	int curr_cmds = commands_in_process++;
	ipc_lowlevel_sendstring (ATTN_STR);
	strcpy (last_cmd, cmd);
	ipc_lowlevel_sendstring (cmd);
	ipc_lowlevel_sendstring (EOL_STR);
	k_sleep(K_MSEC(1));
	if (fwait)
	{
		int cnt = 10;
		while (cnt > 0) 
		{
			if (commands_in_process <= curr_cmds)
			{
				rc = last_error;
				break;
			}
			k_sleep (K_MSEC(100));
			cnt--;
		}
	}
	return rc;
}

/*
 * ipc_echo_cmd - Send a string to coprocessor which will be 
 * echoed back as a command to me. 
 * (Useful for testing)
 */
int ipc_echo_cmd (char *cmd)
{
	char buf[64];
	sprintf (buf, "%s %s", CMD_STR_ECHO, cmd);
	return ipc_cmd_send (buf, false);
}

/*
 * ipc_dbg_out - Send a string to coprocessor which 
 * will be sent out that processor's console.
 */
int ipc_dbg_out (char *str)
{
	char buf[64];
	sprintf (buf, "%s %s", CMD_STR_DBGOUT, str);
	return ipc_cmd_send (buf, false);
}

/*
 * ipc_get_coproc_ver - Queries the version and compile
 * time of the other processor's firmware.
 */
int ipc_get_coproc_ver (char *resp, int siz)
{
	int rc;
	*resp = '\0';
	rc = ipc_cmd_send (CMD_STR_GETVER, true);
	if (rc == 0)
	{
		// Make sure the response is from the getver command.
		if (strncmp (resp_buff, CMD_STR_GETVER, _COUNTOF (CMD_STR_GETVER)-1) == 0)
		{	
			// Copy the data portion of the response.
			strncpy (resp, &resp_buff[_COUNTOF (CMD_STR_GETVER)], siz);
		}
		else
			rc = 5;
	}
	return rc;
}
/*
 * ipc_comm_shutdown - Shutdown the coprocessor communication lib.
 */
int ipc_comm_shutdown (void)
{
	return ipc_lowlevel_shutdown ();
}

/*
 * ipc_init - Initialize the coprocessor communication lib.
 */
int ipc_init (void)
{
	return ipc_init_extended (0, 0);
}
/*
 * ipc_init - Initialize the coprocessor communication lib.
 * 
 * This init function allows adding custom IPC commands specific
 * to the application. 
 */
int ipc_init_extended (struct command_table *p, int command_cnt)
{
	// You can have zero commands, but if you have commands
	// you must point to a command table.
	if ((p == 0) && (command_cnt > 0))
	{
		printk ("Error. Can't have non-zero comamnds with no dispatch table.\n");
		return -1;
	}
	// Save the info
	cmdtblptr = p;
	command_table_cnt = command_cnt;

	// Initialize the CPU specifc code for coprocessor
	// communication.
	int rc = ipc_lowlevel_init (coproc_data_recved);
	//printk ("coproc_cpu_init returned %d\n", rc);
	return rc;
}

