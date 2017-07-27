/****************************************************************************/
/*                                                                 */
/*      Module:         pcx.cpp                                    */
/*                                                                 */
/*      Purpose:        Low-level control of the PC-34 and PC-38 motion control cards   */
/*                                                                 */
/****************************************************************************/
/*                                                                 */
/*                    PROPERTY OF AUTOSCOPE CORPORATION            */
/*                        2637 Midpoint Dr., Suite D               */
/*                         Fort Collins, CO  80525                 */
/*                                                                 */
/*                            Copyright 1995                       */
/*              Unauthorized duplication or use is prohibited.     */
/*                                                                 */
/*      Author:         M. Donahue                                 */
/*                                                                 */
/****************************************************************************/

#include <iostream.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "pcx.h"
#include "io.h"
#include "systimer.h"

/******************************************************************/
/*      local definitions                                                                                                                                                                                       */
/******************************************************************/
// PC-38
#define pc38_data_reg           0x310
#define pc38_done_reg           0x311
#define pc38_control_reg        0x312
#define pc38_status_reg         0x313

// PC-34
#define pc34_data_reg           0x304
#define pc34_done_reg           0x305
#define pc34_control_reg        0x306
#define pc34_status_reg         0x307

// both
#define cmderr_bit              0x01
#define init_bit                0x02
#define enc_bit                 0x04
#define ovrt_bit                0x08
#define done_bit                0x10
#define ibf_bit                 0x20
#define tbe_bit                 0x40
#define irq_bit                 0x80

//------------------------------------------------------------------
//      Name.......:    pcx_check_status
//
//  Purpose....:        check to see if the pcx board is flagging an error or
//                                                              has executed an ID command
//
//      Input......:    card - 1 PC38, 0 PC34
//
//      Output.....:    value contained in the status register
//
//------------------------------------------------------------------
byte pcx_check_status(const byte card)
	{
#ifdef no_pc38
		return(0);
#else
		byte doneflag;
		byte status = inportb(pc38_status_reg);

		if (status & done_bit)
		{
		  if (status & cmderr_bit)
		    cout << "PCX status register indicates a command error\n";
		  else
		    if (status & enc_bit)
		      cout << "PCX status register indicates an encoder slip error\n";
		    else
		      if (status & ovrt_bit)
			cout << "PCX status register indicates limit switch closed\n";
		} else
		{
		  doneflag = ((card) ? inportb(pc38_done_reg) : inportb(pc34_done_reg));
		  if (doneflag) {
		    cout << "PCX status register indicates a done flag set\n";
		    cout << "Axes done is/are: ";
		    if (div(doneflag, 2).rem) cout << "X ";
		    doneflag = div(doneflag, 2).quot;
		    if (div(doneflag, 2).rem) cout << "Y ";
		    doneflag = div(doneflag, 2).quot;
		    if (div(doneflag, 2).rem) cout << "Z ";
		    doneflag = div(doneflag, 2).quot;
		    if (div(doneflag, 2).rem) cout << "T ";
		    doneflag = div(doneflag, 2).quot;
		    if (div(doneflag, 2).rem) cout << "U ";
		    doneflag = div(doneflag, 2).quot;
		    if (div(doneflag, 2).rem) cout << "V ";
		    doneflag = div(doneflag, 2).quot;
		    if (div(doneflag, 2).rem) cout << "R ";
		    doneflag = div(doneflag, 2).quot;
		    if (div(doneflag, 2).rem) cout << "S ";
		    cout << '\n';
		  }
		}

		if (card == PC38)
			{
			while ((inportb(pc38_status_reg) & tbe_bit) != tbe_bit)
				{
					delay(1);
					outportb(pc38_data_reg, 0x18);
				}
			}
		else
			{
			while ((inportb(pc34_status_reg) & tbe_bit) != tbe_bit)
				{
					delay(1);
					outportb(pc34_data_reg, 0x18);
				}
			}

		return status;
#endif
	}


//------------------------------------------------------------------
//      Name.......:    pcx_clear_read_register
//
//  Purpose....:        clear the data register
//
//      Input......:    card - 1 PC38, 0 PC34
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_clear_read_register(const byte card)
	{
		byte v=1;
		while (v) {
		  v = pcx_read_byte(card);
		//  printf("clear: %c ",v);
		}
		//while (pcx_read_byte(card));
	}

//------------------------------------------------------------------
//      Name.......:    pcx_clear_done_flags
//
//  Purpose....:  clear the done flag(s) on the given axis/axes
//
//      Input......:  card - 1 PC38, 0 PC34
//                                                              axes - one or more axis constants ORed together
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_clear_done_flags(const byte card, const byte axes)
	{
#ifdef no_pc38
		SysTimer[FAKEMOVE].NewTimerSecs(4);
		return;
#else
		if (axes & pcx_x_axis)
			pcx_send_commands(card, "ax ca;");

		if (axes & pcx_y_axis)
			pcx_send_commands(card, "ay ca;");

		if (axes & pcx_z_axis)
			pcx_send_commands(card, "az ca;");

		if (axes & pcx_t_axis)
			pcx_send_commands(card, "at ca;");

		if (axes & pcx_u_axis)
			pcx_send_commands(card, "au ca;");

		if (axes & pcx_v_axis)
			pcx_send_commands(card, "av ca;");
#endif
	}

//------------------------------------------------------------------
//      Name.......:    pcx_disable_interrupts
//
//  Purpose....:        disable the IRQ and use a polled method
//
//      Input......:    card - 1 PC38, 0 PC34
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_disable_interrupts(const byte card)
	{
#ifdef no_pc38
		return;
#else
		if (card == PC38)
			outportb(pc38_control_reg, 0);
		else
			outportb(pc34_control_reg, 0);
#endif
	}

//------------------------------------------------------------------
//      Name.......:    pcx_done
//
//  Purpose....:        determine the done state of an axis/axes
//
//      Input......:  card - 1 PC38, 0 PC34
//                                                              axes - one or more axis constants ORed together
//
//      Output.....:    TRUE if the axis is done
//
//------------------------------------------------------------------
BOOL pcx_done(const byte card, const byte axes)
	{
		AXISPACK axisData;
#ifdef no_pc38
		if (!SysTimer[FAKEMOVE].Expired()) 
		  return(FALSE);
		else
		  return(TRUE);
#else

		// mask out unused axes
		BOOL x_done = !(axes & pcx_x_axis);
		BOOL y_done = !(axes & pcx_y_axis);
		BOOL z_done = !(axes & pcx_z_axis);
		BOOL t_done = !(axes & pcx_t_axis);
		BOOL u_done = !(axes & pcx_u_axis);
		BOOL v_done = !(axes & pcx_v_axis);

		if (!x_done)
			{
				pcx_get_axis_status(card, 'x', axisData);
				x_done = axisData.done;
			}

		if (!y_done)
			{
				pcx_get_axis_status(card, 'y', axisData);
				y_done = axisData.done;
			}

		if (!z_done)
			{
				pcx_get_axis_status(card, 'z', axisData);
				z_done = axisData.done;
			}

		if (!t_done)
			{
				pcx_get_axis_status(card, 't', axisData);
				t_done = axisData.done;
			}

		if (!u_done)
			{
				pcx_get_axis_status(card, 'u', axisData);
				u_done = axisData.done;
			}

		if (!v_done)
			{
				pcx_get_axis_status(card, 'v', axisData);
				v_done = axisData.done;
			}

		// return the result
		return (x_done && y_done && z_done && t_done && u_done && v_done);
#endif
	}

//------------------------------------------------------------------
//      Name.......:    pcx_get_axis_status
//
//  Purpose....:        return the 4 axis status indicators for the given axis
//
//      Input......:  card - 1 PC38, 0 PC34
//                 axis - one of the following axis characters: X, Y, Z, T, U, V
//
//      Output.....:    axisRec - axis data (movement dir, done, limit, home)
//
//------------------------------------------------------------------
void pcx_get_axis_status(const byte card, const char axis,
			 struct AXISPACK &axisData)
	{
		char buffer[80];
		int len;
		char c1, c2, c3, c4;
#ifdef no_pc38
		return;
#else

		strcpy(buffer,"a");
		len = strlen(buffer);
		buffer[len++] = axis;
		buffer[len] = 0;
		strcat(buffer, " QA");

		// get the status
		pcx_send_commands(card, buffer);

		do
			c1 = pcx_read_byte(card);
		while ((c1 != 'P') && (c1 != 'M'));
		axisData.posdir = (c1 == 'P');

		do
			c2 = pcx_read_byte(card);
		while ((c2 != 'D') && (c2 != 'N'));
		axisData.done = (c2 == 'D');

		do
			c3 = pcx_read_byte(card);
		while ((c3 != 'L') && (c3 != 'N'));
		axisData.limit = (c3 == 'L');

		do
			c4 = pcx_read_byte(card);
		while ((c4 != 'H') && (c4 != 'N'));
		axisData.home = (c4 == 'H');
#endif
	}

//------------------------------------------------------------------
//      Name.......:    pcx_read_byte
//
//  Purpose....:        returns the current byte available
//
//      Input......:    card - 1 PC38, 0 PC34
//
//      Output.....:    \x00 for no char, or character ready
//
//------------------------------------------------------------------
byte pcx_read_byte(const byte card)
	{
	byte stat,data;
#ifdef no_pc38
		return(0);
#else
	if (card == PC38){
		stat= ((inportb(pc38_status_reg) & ibf_bit) ?
		data=inportb(pc38_data_reg) : 0);
	//	printf("%c %c\n",stat,data);
	    return(stat ? data : 0);
	//	return ((inportb(pc38_status_reg) & ibf_bit) ?
//			inportb(pc38_data_reg) : 0);
    }
	else
		return ((inportb(pc34_status_reg) & ibf_bit) ?
			inportb(pc34_data_reg) : 0);
#endif
	}

//------------------------------------------------------------------
//      Name.......:    pcx_read_status_reg
//
//  Purpose....:        returns the current status register value
//
//      Input......:    card - 1 PC38, 0 PC34
//
//      Output.....:    status register value
//
//------------------------------------------------------------------
byte pcx_read_status_reg(const byte card)
	{
#ifdef no_pc38
		return(0);
#else
		if (card == PC38)
			return inportb(pc38_status_reg);
		else
			return inportb(pc34_status_reg);
#endif
	}

//------------------------------------------------------------------
//      Name.......:    pcx_send_commands
//
//  Purpose....:        send a command string to the PCX card
//
//      Input......:    card - 1 PC38, 0 PC34
//                                                              commands - string of commands to send
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_send_commands(const byte card, const char *commands)
	{
		if (!strlen(commands))
			return;
#ifdef no_pc38
		return;
#else

		BOOL do_echo = (commands[0] == 1);      // \x01 means echo the command
		int idx = (int)do_echo;                                                 // if echo, skip first byte
		char *msg = strupr(strdup(commands));

		do
			{
				pcx_write_byte(card, msg[idx]);
				if (do_echo)
					cout << msg[idx];
			}
		while (msg[++idx]);
		if (do_echo)
			cout << endl;
		pcx_clear_read_register(card);
		free(msg);
#endif
	}

//------------------------------------------------------------------
//      Name.......:    pcx_write_byte
//
//  Purpose....:        send a byte to the PCX card data register
//
//      Input......:    card - 1 PC38, 0 PC34
//                                                              outByte - byte to write
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_write_byte(const byte card, const byte outByte)
	{
#ifdef no_pc38
		return;
#else
		// wait until it is clear to transmit
		if (card == PC38)
			while ((inportb(pc38_status_reg) & tbe_bit) != tbe_bit);
		else
			while ((inportb(pc34_status_reg) & tbe_bit) != tbe_bit);

		// introduce a 100 microsecond delay to accomodate some of our
		// computers.  Their busses aren't the greatest.  Use NOPs
		for (int i = 0; i < 500; i++)
			asm
				{
					nop;
					nop;
					nop;
					nop;
					nop;
				}

		if (card == PC38)
			outportb(pc38_data_reg, outByte);
		else
			outportb(pc34_data_reg, outByte);
#endif
	}

//------------------------------------------------------------------
//      Name.......:    pcx_write_control
//
//  Purpose....:        send a byte to the PCX card control register
//
//      Input......:    card - 1 PC38, 0 PC34
//                                                              outByte - byte to write
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_write_control(const byte card, const byte outByte)
	{
#ifdef no_pc38
		return;
#else
		if (card == PC38)
			outportb(pc38_control_reg, outByte);
		else
			outportb(pc34_control_reg, outByte);
#endif
	}

/********************************* EOF **************************************/

unsigned pcx_read_inputs(const byte card)
        {
                #ifdef no_pc38
                return 0;
                #else

                byte v = 0;
                char hexval[3];

        // clear the read register and send the BX command to get the bit value
                pcx_clear_read_register(card);
                pcx_send_commands(card,"bx");

                // the number comes back as <lf><cr>##<lf><cr>
                while (v != 10)
                        v = pcx_read_byte(card);

                while (v != 13)
                        v = pcx_read_byte(card);

                v = pcx_read_byte(card);
                while (!v)
                        v = pcx_read_byte(card);
                hexval[0] = v;

                v = pcx_read_byte(card);
                while (!v)
                        v = pcx_read_byte(card);
                hexval[1] = v;
                hexval[2] = 0;

                v = pcx_read_byte(card);
                while (v != 10)
                        v = pcx_read_byte(card);

                while (v != 13)
                        v = pcx_read_byte(card);
                // convert hexval to an integer
                char hexdigits[] = "0123456789ABCDEFabcdef";
                unsigned rval = 0;

                for (byte i = 0; i < 2; i++)
                   for (v = 0; v < 22; v++)
                      if (hexval[i] == hexdigits[v])
                        rval = (16 * rval) + ((v < 16) ? v : v - 6);

                return rval;
                #endif
        }
unsigned pc38_read_inputs()
        {
                #ifdef no_pc38
                return 0;
                #else

		int j;
                byte v = 0;
                char hexval[7];

        // clear the read register and send the BX command to get the bit value
                pc38_clear_read_register();
                pc38_send_commands("bx");

                // the number comes back as <lf><cr>##<lf><cr>
                while (v != 10) {
                  v = pc38_read_byte();
		  //fprintf(stderr,"v1: %c ",v);
		}

                while (v != 13){
                  v = pc38_read_byte();
		  //fprintf(stderr,"v2: %c ",v);
		}

	        for (j=0 ; j<6 ; j++) {
                  v = pc38_read_byte();
                  while (!v) {
		    //fprintf(stderr,"v%d: %c ",j,v);
                    v = pc38_read_byte();
	          }
                  hexval[j] = v;
	        }
                hexval[6] = 0;

                v = pc38_read_byte();
                while (v != 10) {
		  //fprintf(stderr,"v5: %c ",v);
                  v = pc38_read_byte();
		}

                while (v != 13) {
		  //fprintf(stderr,"v6: %c ",v);
                  v = pc38_read_byte();
		}
		//fprintf(stderr,"\nhexval: %s\n",hexval);

                // convert hexval to an integer
                char hexdigits[] = "0123456789ABCDEFabcdef";
                unsigned rval = 0;

                for (byte i = 0; i < 6; i++)
                   for (v = 0; v < 22; v++)
                      if (hexval[i] == hexdigits[v])
                        rval = (16 * rval) + ((v < 16) ? v : v - 6);

                return rval;
                #endif
        }

