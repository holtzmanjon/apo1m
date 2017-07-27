/******************************************************************************
*
*   File:       DSPCommand.c
*   Version:    1.00
*   Author:     Scott Streit
*   Abstract:   C library for DSP functions.
*
*   Description:
*		Non-Vector commands are sent in the following order:
*		header = 0xssddnn
*			ss = source byte = 0 
*			dd = destination byte 
*				= 1 for PCI board
*				= 2 for timing board
*				= 3 for utility board
*			nn = number of words in command (>=2)
*				= (header + command + number_of_arguments) >= 2
*		command = 24 bit ASCII
*		argument1 (optional)
*		argument2 (optional)
*		argument3 (optional)
*		argument4 (optional)
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       11/22/99        sds     1.00    Initial
*
*       06/20/00        jmp     1.00    Added support for global variables:
*										reply_value, str_reply_value,
*										arg_and_board_dest.
*										Update functions and add new ones.
*										(see functions).
*
* 		03/22/2001		sds		1.60    Upgraded commands to version 1.6.
*
*		01/03/2002		sds		1.7		Modified to be win2k static library.
*
******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <sys/ioctl.h>
#include "DSPCommand.h"
#include "../WhichSystem.h"


#ifdef WIN2K
	#include <windows.h>
	#include <winioctl.h>
#else
	#include <sys/mman.h>
	#include <fcntl.h>
#endif

/* Actual error returned from hardware */
int error = 0;

/* Command array */
#define MAX_DATA	6
int cmd_data[MAX_DATA];

/**
*****************************************************************************
*	Get the current HSTR value.
*****************************************************************************
*/
#ifdef WIN2K
int getHstr(HANDLE pci_fd)
{
	int reply = 0;
	ULONG bytesReturned;

	/* Get the current HCTR value. */
	DeviceIoControl(pci_fd, ASTROPCI_GET_HSTR, NULL, 0, &reply, sizeof(reply),
						&bytesReturned, NULL);

	return reply;
}

#else
int getHstr(int pci_fd)
{
	int reply = 0;
	
		/* Get the current HCTR value. */
	ioctl(pci_fd, ASTROPCI_GET_HSTR, &reply);
	
	return reply;
}
#endif

/**
*****************************************************************************
*	Send a command to the PCI DSP via the vector register. The reply
*	returned from the PCI DSP is checked against the specified expected
*	reply value.
*
*	board_id	The board to receive the command. Can be one of: PCI_ID,
*		 	TIM_ID, or UTIL_ID.
*	command		The vector command to be sent to the PCI DSP.
*	data		The data associated with the vector command.
*	expected_reply	The value of the expected reply.
*****************************************************************************
*/
#ifdef WIN2K
int hcvr(HANDLE pci_fd, int command, int data, int expected_reply)
{
	int reply = _NO_ERROR;
	ULONG bytesReturned;

	/* If there's data, send it */
	if (data != UNDEFINED)
	DeviceIoControl(pci_fd, ASTROPCI_HCVR_DATA, &data, sizeof(data), NULL, 0,
						&bytesReturned, NULL);

	/* Send the command */
	DeviceIoControl(pci_fd, ASTROPCI_SET_HCVR, &command, sizeof(command),
						&reply, sizeof(reply), &bytesReturned, NULL);

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}

#else
int hcvr(int pci_fd, int command, int data, int expected_reply)
{
	int reply = _NO_ERROR;

	/* If there's data, send it */
	if (data != UNDEFINED)
		ioctl(pci_fd, ASTROPCI_HCVR_DATA, &data);
		
	/* Send the command */
	ioctl(pci_fd, ASTROPCI_SET_HCVR, &command);
	reply = command;

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}
#endif

/**
*****************************************************************************
*	Send a command to the PCI DSP via the vector register. The reply
*	returned from the PCI DSP is checked against the specified expected
*	reply value.
*
*	board_id	The board to receive the command. Can be one of: PCI_ID,
*		 	TIM_ID, or UTIL_ID.
*	command		The vector command to be sent to the PCI DSP.
*	data1		The first data value associated with the vector command.
*	data2		The second data value associated with the vector command.
*	expected_reply	The value of the expected reply.
*****************************************************************************
*/
#ifdef WIN2K
int hcvr2(HANDLE pci_fd, int command, int data1, int data2, int expected_reply)
{
	int reply = _NO_ERROR;
	ULONG bytesReturned;

	/* If there's data, send it */
	if (data1 != UNDEFINED)
		DeviceIoControl(pci_fd, ASTROPCI_HCVR_DATA, &data1, sizeof(data1),
							NULL, 0, &bytesReturned, NULL);

	if (data2 != UNDEFINED)
		DeviceIoControl(pci_fd, ASTROPCI_HCVR_DATA, &data2, sizeof(data2),
							NULL, 0, &bytesReturned, NULL);

	/* Send the command */
	DeviceIoControl(pci_fd, ASTROPCI_SET_HCVR, &command, sizeof(command),
						&reply, sizeof(reply), &bytesReturned, NULL);

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}

#else
int hcvr2(int pci_fd, int command, int data1, int data2, int expected_reply)
{
	int reply = _NO_ERROR;

	/* If there's data, send it */
	if (data1 != UNDEFINED)
		ioctl(pci_fd, ASTROPCI_HCVR_DATA, &data1);
	if (data2 != UNDEFINED)
		ioctl(pci_fd, ASTROPCI_HCVR_DATA, &data2);
		
	/* Send the command */
	ioctl(pci_fd, ASTROPCI_SET_HCVR, &command);
	reply = command;

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}
#endif

/**
*****************************************************************************
*	Send a command with a data value to the specified board. The reply
*	returned from the PCI DSP is checked against the specified expected
*	reply value.
*
*	board_id	The board to receive the command. Can be one of: PCI_ID,
*		 	TIM_ID, or UTIL_ID.
*	command		The command to be sent to the PCI DSP.
*	expected_reply	The value of the expected reply.
*****************************************************************************
*/
#ifdef WIN2K
int doCommand(HANDLE pci_fd, int board_id, int command, int expected_reply)
{
	int reply = _NO_ERROR;
	ULONG bytesReturned;
	
	cmd_data[0] = ((board_id << 8) | 2);
	cmd_data[1] = command;
	cmd_data[2] = UNDEFINED;
	cmd_data[3] = UNDEFINED;
	cmd_data[4] = UNDEFINED;
	cmd_data[5] = UNDEFINED;

	/* Send the command */
	DeviceIoControl(pci_fd, ASTROPCI_COMMAND, &cmd_data, sizeof(cmd_data),
						&reply, sizeof(reply), &bytesReturned, NULL);

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}

#else
int doCommand(int pci_fd, int board_id, int command, int expected_reply)
{
	int reply = _NO_ERROR;
	
	cmd_data[0] = ((board_id << 8) | 2);
	cmd_data[1] = command;
	cmd_data[2] = UNDEFINED;
	cmd_data[3] = UNDEFINED;
	cmd_data[4] = UNDEFINED;
	cmd_data[5] = UNDEFINED;

	/* Send the command */
	ioctl(pci_fd, ASTROPCI_COMMAND, &cmd_data);
	reply = cmd_data[0];

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}
#endif

/**
*****************************************************************************
*	Send a command with a data value to the specified board. The reply
*	returned from the PCI DSP is checked against the specified expected
*	reply value.
*
*	board_id	The board to receive the command. Can be one of: PCI_ID,
*		 	TIM_ID, or UTIL_ID.
*	command		The command to be sent to the PCI DSP.
*	arg1		Optional argument.
*	expected_reply	The value of the expected reply.
*****************************************************************************
*/
#ifdef WIN2K
int doCommand1(HANDLE pci_fd, int board_id, int command, int arg1, int expected_reply)
{
	int reply = _NO_ERROR;
	ULONG bytesReturned;

	cmd_data[0] = ((board_id << 8) | 3);
	cmd_data[1] = command;
	cmd_data[2] = arg1;
	cmd_data[3] = UNDEFINED;
	cmd_data[4] = UNDEFINED;
	cmd_data[5] = UNDEFINED;

	/* Send the command */
	DeviceIoControl(pci_fd, ASTROPCI_COMMAND, &cmd_data, sizeof(cmd_data),
						&reply, sizeof(reply), &bytesReturned, NULL);

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}

#else
int doCommand1(int pci_fd, int board_id, int command, int arg1, int expected_reply)
{
	int reply = _NO_ERROR;

	cmd_data[0] = ((board_id << 8) | 3);
	cmd_data[1] = command;
	cmd_data[2] = arg1;
	cmd_data[3] = UNDEFINED;
	cmd_data[4] = UNDEFINED;
	cmd_data[5] = UNDEFINED;

	/* Send the command */
	ioctl(pci_fd, ASTROPCI_COMMAND, &cmd_data);
	reply = cmd_data[0];

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}
#endif

/**
*****************************************************************************
*	Send a command with a data value to the specified board. The reply
*	returned from the PCI DSP is checked against the specified expected
*	reply value.
*
*	board_id	The board to receive the command. Can be one of: PCI_ID,
*		 	TIM_ID, or UTIL_ID.
*	command		The command to be sent to the PCI DSP.
*	arg1		Optional argument.
*	arg2		Optional argument.
*	expected_reply	The value of the expected reply.
*****************************************************************************
*/
#ifdef WIN2K
int doCommand2(HANDLE pci_fd, int board_id, int command, int arg1, int arg2, int expected_reply)
{
	int reply = _NO_ERROR;
	ULONG bytesReturned;

	cmd_data[0] = ((board_id << 8) | 4);
	cmd_data[1] = command;
	cmd_data[2] = arg1;
	cmd_data[3] = arg2;
	cmd_data[4] = UNDEFINED;
	cmd_data[5] = UNDEFINED;

	/* Send the command */
	DeviceIoControl(pci_fd, ASTROPCI_COMMAND, &cmd_data, sizeof(cmd_data),
						&reply, sizeof(reply), &bytesReturned, NULL);

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}

#else
int doCommand2(int pci_fd, int board_id, int command, int arg1, int arg2, int expected_reply)
{
	int reply = _NO_ERROR;

	cmd_data[0] = ((board_id << 8) | 4);
	cmd_data[1] = command;
	cmd_data[2] = arg1;
	cmd_data[3] = arg2;
	cmd_data[4] = UNDEFINED;
	cmd_data[5] = UNDEFINED;

	/* Send the command */
	ioctl(pci_fd, ASTROPCI_COMMAND, &cmd_data);
	reply = cmd_data[0];

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}
#endif

/**
*****************************************************************************
*	Send a command with a data value to the specified board. The reply
*	returned from the PCI DSP is checked against the specified expected
*	reply value.
*
*	board_id	The board to receive the command. Can be one of: PCI_ID,
*		 	TIM_ID, or UTIL_ID.
*	command		The command to be sent to the PCI DSP.
*	arg1		Optional argument.
*	arg2		Optional argument.
*	arg3		Optional argument.
*	expected_reply	The value of the expected reply.
*****************************************************************************
*/
#ifdef WIN2K
int doCommand3(HANDLE pci_fd, int board_id, int command, int arg1, int arg2, int arg3, int expected_reply)
{
	int reply = _NO_ERROR;
	ULONG bytesReturned;

	cmd_data[0] = ((board_id << 8) | 5);
	cmd_data[1] = command;
	cmd_data[2] = arg1;
	cmd_data[3] = arg2;
	cmd_data[4] = arg3;
	cmd_data[5] = UNDEFINED;

	/* Send the command */
	DeviceIoControl(pci_fd, ASTROPCI_COMMAND, &cmd_data, sizeof(cmd_data),
						&reply, sizeof(reply), &bytesReturned, NULL);

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}

#else
int doCommand3(int pci_fd, int board_id, int command, int arg1, int arg2, int arg3, int expected_reply)
{
	int reply = _NO_ERROR;

	cmd_data[0] = ((board_id << 8) | 5);
	cmd_data[1] = command;
	cmd_data[2] = arg1;
	cmd_data[3] = arg2;
	cmd_data[4] = arg3;
	cmd_data[5] = UNDEFINED;

	/* Send the command */
	ioctl(pci_fd, ASTROPCI_COMMAND, &cmd_data);
	reply = cmd_data[0];

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}
#endif

/**
*****************************************************************************
*	Send a command with a data value to the specified board. The reply
*	returned from the PCI DSP is checked against the specified expected
*	reply value.
*
*	board_id	The board to receive the command. Can be one of: PCI_ID,
*		 	TIM_ID, or UTIL_ID.
*	command		The command to be sent to the PCI DSP.
*	arg1		Optional argument.
*	arg2		Optional argument.
*	arg3		Optional argument.
*	arg4		Optional argument.
*	expected_reply	The value of the expected reply.
*****************************************************************************
*/
#ifdef WIN2K
int doCommand4(HANDLE pci_fd, int board_id, int command, int arg1, int arg2, int arg3, int arg4, int expected_reply)
{
	int reply = _NO_ERROR;
	ULONG bytesReturned;

	cmd_data[0] = ((board_id << 8) | 6);
	cmd_data[1] = command;
	cmd_data[2] = arg1;
	cmd_data[3] = arg2;
	cmd_data[4] = arg3;
	cmd_data[5] = arg4;

	/* Send the command */
	DeviceIoControl(pci_fd, ASTROPCI_COMMAND, &cmd_data, sizeof(cmd_data),
						&reply, sizeof(reply), &bytesReturned, NULL);

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}

#else
int doCommand4(int pci_fd, int board_id, int command, int arg1, int arg2, int arg3, int arg4, int expected_reply)
{
	int reply = _NO_ERROR;

	cmd_data[0] = ((board_id << 8) | 6);
	cmd_data[1] = command;
	cmd_data[2] = arg1;
	cmd_data[3] = arg2;
	cmd_data[4] = arg3;
	cmd_data[5] = arg4;

	/* Send the command */
	ioctl(pci_fd, ASTROPCI_COMMAND, &cmd_data);
	reply = cmd_data[0];

	/* If a reply is expected, check it */
	if (expected_reply != UNDEFINED)
		return (check_expected_reply(reply, expected_reply));
	else
		return (check_standard_reply(reply));
}
#endif

/**
*****************************************************************************
*	Return the error.
*****************************************************************************
*/
int getError()
{
	return error;
}

/**
*****************************************************************************
*	Check the reply returned from the PCI DSP against the specified
*	expected reply value.
*
*	reply		Actual reply from PCI.
*	expected_reply	The value of the expected reply.
*****************************************************************************
*/
int check_expected_reply(int reply, int expected_reply)
{
	if (reply != expected_reply) {
		error = reply;
		return _ERROR;
	}
	else
		return _NO_ERROR;
}

/**
*****************************************************************************
*	Check the reply returned from the PCI DSP against the standard set
*	of replies: ERR, SYR, RST, TOUT. If the reply does not match one of
*	the standard ones, the reply is just returned.
*
*	reply		Actual reply from PCI.
*****************************************************************************
*/
int check_standard_reply(int reply)
{
	switch (reply) {
		case ERR:
		case SYR:
		case RST:
		case TOUT:
			error = reply; 
			return _ERROR;
				
		default:
			return reply;
	}
}
