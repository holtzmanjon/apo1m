/****************************************************************************/
/*                                                                 */
/* Module:   pcx.cpp                                               */
/*                                                                 */
/* Purpose:  Low-level control of the PC-34 and PC-38 motion control cards  */
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
/*      Modifications by:  Jon Holtzman , New Mexico State University */
/****************************************************************************/

#include <iostream.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "pcx.h"
#include "tcs.h"
#include "globals.h"
#include "io.h"
#include "systimer.h"

/******************************************************************/
/*      local definitions                                         */
/******************************************************************/
// Telescope PC-38
#define pc38_data_reg           0x310
#define pc38_done_reg           0x311
#define pc38_control_reg        0x312
#define pc38_status_reg         0x313
//#define pc38_data_reg           0x300
//#define pc38_done_reg           0x301
//#define pc38_control_reg        0x302
//#define pc38_status_reg         0x303

// Dome PC-34
#define pc34_data_reg           0x314
#define pc34_done_reg           0x315
#define pc34_control_reg        0x316
#define pc34_status_reg         0x317

// Guider/filter wheel PC-38
#define guiderpc38_data_reg         0x318
#define guiderpc38_done_reg         0x319
#define guiderpc38_control_reg      0x31a
#define guiderpc38_status_reg       0x31b

// both
#define cmderr_bit              0x01
#define init_bit                0x02
#define enc_bit                 0x04
#define ovrt_bit                0x08
#define done_bit                0x10
#define ibf_bit                 0x20
#define tbe_bit                 0x40
#define irq_bit                 0x80

char tmpout[300];

//------------------------------------------------------------------
//      Name.......:    pcx_check_status
//
//  Purpose....:        check to see if the pcx board is flagging an error or
//                         has executed an ID command
//
//      Input......:    card - 1 PC38, 0 PC34
//
//      Output.....:    value contained in the status register
//
//------------------------------------------------------------------
byte pcx_check_status(const byte card)
{
#ifdef no_hardware
  return(0);
#else
  byte doneflag;
  byte status;
  int icount;

  if (card == PC38)
    status = inportb(pc38_status_reg);
  else if (card == PC34)
    status = inportb(pc34_status_reg);
  else if (card == GUIDERPC38)
    status = inportb(guiderpc38_status_reg);

  if (status & done_bit) {
    if (status & cmderr_bit)
	    cout << "PCX status register indicates a command error\n";
    else if (status & enc_bit)
	    cout << "PCX status register indicates an encoder slip error\n";
    else if (status & ovrt_bit)
	    cout << "PCX status register indicates limit switch closed\n";
  } else {
     if (card == PC38 )
       doneflag = inportb(pc38_done_reg);
     else if (card == PC34)
       doneflag = inportb(pc34_done_reg);
     else if (card == GUIDERPC38)
       doneflag = inportb(guiderpc38_done_reg);

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

  icount=0;
  if (card == PC38) {
    while ((inportb(pc38_status_reg) & tbe_bit) != tbe_bit && icount<1000) {
	delay(1);
	outportb(pc38_data_reg, 0x18);
        icount++;
    }
  } else if (card == PC34) {
    while ((inportb(pc34_status_reg) & tbe_bit) != tbe_bit && icount<1000) {
	delay(1);
	outportb(pc34_data_reg, 0x18);
        icount++;
    }
  } else if (card == GUIDERPC38) {
    while ((inportb(guiderpc38_status_reg) & tbe_bit) != tbe_bit && icount<1000) {
	delay(1);
	outportb(guiderpc38_data_reg, 0x18);
        icount++;
    }
  }
  if ( icount == 1000) {
    sprintf(tmpout,"error in pcx_check_status: %d",card);
    writeline(tmpout,1);
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
  int icount;
  icount=0;
  while (pcx_read_byte(card,0) && icount<1000) {
   icount++;
  }
  if (icount == 1000) {
     sprintf(tmpout,"Error in clear_read_register");
     writeline(tmpout,1);
     if (card==PC38) {
       tcs_set_deinit_telescope();
       tcs_telescope_stop(FALSE);
     }
  }
}

//------------------------------------------------------------------
//      Name.......:    pcx_clear_done_flags
//
//  Purpose....:  clear the done flag(s) on the given axis/axes
//
//      Input......:  card - 1 PC38, 0 PC34
//         axes - one or more axis constants ORed together
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_clear_done_flags(const byte card, const byte axes)
{
#ifdef no_hardware
  SysTimer[FAKEMOVE].NewTimerSecs(4);
  return;
#else
  if (axes & pcx_x_axis) pcx_send_commands(card, "ax ca;");

  if (axes & pcx_y_axis) pcx_send_commands(card, "ay ca;");

  if (axes & pcx_z_axis) pcx_send_commands(card, "az ca;");

  if (axes & pcx_t_axis) pcx_send_commands(card, "at ca;");

  if (axes & pcx_u_axis) pcx_send_commands(card, "au ca;");

  if (axes & pcx_v_axis) pcx_send_commands(card, "av ca;");
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
#ifdef no_hardware
  return;
#else
  if (card == PC38) 
     outportb(pc38_control_reg, 0);
  else if (card == PC34)
     outportb(pc34_control_reg, 0);
  else if (card == GUIDERPC38)
     outportb(guiderpc38_control_reg, 0);
  delay(500);
#endif
}

//------------------------------------------------------------------
//      Name.......:    pcx_done
//
//  Purpose....:        determine the done state of an axis/axes
//
//      Input......:  card - 1 PC38, 0 PC34
//                    axes - one or more axis constants ORed together
//
//      Output.....:    TRUE if the axis is done
//
//------------------------------------------------------------------
BOOL pcx_done(const byte card, const byte axes)
{
  AXISPACK axisData;
#ifdef no_hardware
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

  if (!x_done) {
    pcx_get_axis_status(card, 'x', axisData);
    x_done = axisData.done;
  }

  if (!y_done) {
    pcx_get_axis_status(card, 'y', axisData);
    y_done = axisData.done;
  }

  if (!z_done) {
    pcx_get_axis_status(card, 'z', axisData);
    z_done = axisData.done;
  }

  if (!t_done) {
    pcx_get_axis_status(card, 't', axisData);
    t_done = axisData.done;
  }

  if (!u_done) {
    pcx_get_axis_status(card, 'u', axisData);
    u_done = axisData.done;
  }

  if (!v_done) {
    pcx_get_axis_status(card, 'v', axisData);
    v_done = axisData.done;
  }

  // return the result
  return (x_done && y_done && z_done && t_done && u_done && v_done);
#endif
}

//------------------------------------------------------------------
//  Name.......:    pcx_get_axis_status
//
//  Purpose....:    return the 4 axis status indicators for the given axis
//
//  Input......:card - 1 PC38, 0 PC34
//              axis - one of the following axis characters: X, Y, Z, T, U, V
//
//  Output.....:    axisRec - axis data (movement dir, done, limit, home)
//
//------------------------------------------------------------------
void pcx_get_axis_status(const byte card, const char axis,
		         struct AXISPACK &axisData)
{
  char buffer[80];
  int len, icount;
  char c1, c2, c3, c4;
#ifdef no_hardware
  return;
#else

//  strcpy(buffer,"a");
//  len = strlen(buffer);
//  buffer[len++] = axis;
//  buffer[len] = 0;
//  strcat(buffer, " QA");

  sprintf(buffer,"a%c QA",axis);

  // get the status
  pcx_send_commands(card, buffer);

  icount=0;  
  do {
    c1 = pcx_read_byte(card,0);
    icount++;
  } while ((c1 != 'P') && (c1 != 'M') && icount<1000);
  axisData.posdir = (c1 == 'P');

  do {
    c2 = pcx_read_byte(card,0);
    icount++;
  } while ((c2 != 'D') && (c2 != 'N') && icount<2000);
  axisData.done = (c2 == 'D');

  do {
    c3 = pcx_read_byte(card,0);
    icount++;
  } while ((c3 != 'L') && (c3 != 'N') && icount<3000);
  axisData.limit = (c3 == 'L');

  do {
    c4 = pcx_read_byte(card,0);
    icount++;
  } while ((c4 != 'H') && (c4 != 'N') && icount<4000);
  axisData.home = (c4 == 'H');
  
  if (icount>1000) {
    sprintf(tmpout,"error in get_axis_status: %d %d %c %c %c %c",card,icount,c1,c2,c3,c4);
    writeline(tmpout,1);
  }
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
byte pcx_read_byte(const byte card, int show)
{
  byte out, status;
  int icount, status_reg, data_reg;
  int maxcount=1000;
 
#ifdef no_hardware
  return(0);
#else
  if (card == PC38) {
    status_reg=pc38_status_reg;
    data_reg=pc38_data_reg;
  } else if (card == PC34) {
    status_reg=pc34_status_reg;
    data_reg=pc34_data_reg;
  } else if (card == GUIDERPC38) {
    status_reg=guiderpc38_status_reg;
    data_reg=guiderpc38_data_reg;
  }
  icount=0;
  if (show) {
    sprintf(tmpout,"checking init");
    writeline(tmpout,1);
  }
  while (((status=inportb(status_reg)) & init_bit) == init_bit && icount<maxcount) {
    sprintf(tmpout,"init:  %x",status);
    writeline(tmpout,1);
    delay(1);
    icount++;
  }
  if (icount == maxcount) {
    sprintf(tmpout,"error in pcx_read_byte: %d",card);
    writeline(tmpout,1);
  }
  icount=0;
  if (show) {
    sprintf(tmpout,"checking ibf");
    writeline(tmpout,1);
  }
  while (((status=inportb(status_reg)) & ibf_bit) != ibf_bit && icount<maxcount) {
    //sprintf(tmpout,"ibf:  %x",status);
    //writeline(tmpout,1);
    //delay(1);
    icount++;
  }
  if (show) {
    sprintf(tmpout,"done ibf: %d",icount);
    writeline(tmpout,1);
  }
  // If there's nothing to read, return 0
  if (icount == maxcount) {
    if (show) {
      sprintf(tmpout,"nothing to read: %d",card);
      writeline(tmpout,1);
    }
    return(0);
  }
  if (show) {
    sprintf(tmpout,"inportb");
    writeline(tmpout,1);
  }
  out = inportb(data_reg);
  if (show) {
    sprintf(tmpout,"done inportb: %d",out);
    writeline(tmpout,1);
  }

/*
  if (card == PC38) {
    out= ((inportb(pc38_status_reg) & ibf_bit) ?
			inportb(pc38_data_reg) : 0);
  } else if (card == PC34) {
    out= ((inportb(pc34_status_reg) & ibf_bit) ?
			inportb(pc34_data_reg) : 0);
  } else if (card == GUIDERPC38) {
    out= ((inportb(guiderpc38_status_reg) & ibf_bit) ?
			inportb(guiderpc38_data_reg) : 0);
  }
*/
  //sprintf(tmpout,"%c ",out);
  //writeline(tmpout,1);
  return(out);
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
#ifdef no_hardware
  return(0);
#else
  if (card == PC38)
    return inportb(pc38_status_reg);
  else if (card == PC34)
    return inportb(pc34_status_reg);
  else if (card == GUIDERPC38)
    return inportb(guiderpc38_status_reg);
  return(0);
#endif
}

//------------------------------------------------------------------
//      Name.......:    pcx_send_commands
//
//  Purpose....:        send a command string to the PCX card
//
//      Input......:    card - 1 PC38, 0 PC34
//                      commands - string of commands to send
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_send_commands(const byte card, const char *commands)
{
  //sprintf(tmpout,"pcx_send_commands %d %s\n",card,commands);
  //writeline(tmpout,1);
  if (!strlen(commands)) return;
#ifdef no_hardware
  return;
#else

  BOOL do_echo = (commands[0] == 1);      // \x01 means echo the command
  int idx = (int)do_echo;                 // if echo, skip first byte
  char *msg = strupr(strdup(commands));

  //sprintf(tmpout,"pcx_send_commands %d %s\n",card,commands);
  //writeline(tmpout,1);
  do {
    pcx_write_byte(card, msg[idx]);
    if (do_echo) cout << msg[idx];
  } while (msg[++idx]);
  if (do_echo) cout << endl;
//  pcx_clear_read_register(card);
  //sprintf(tmpout,"done pcx_send_commands %d %s\n",card,commands);
  //writeline(tmpout,1);
  free(msg);
#endif
}

//------------------------------------------------------------------
//      Name.......:    pcx_write_byte
//
//  Purpose....:        send a byte to the PCX card data register
//
//      Input......:    card - 1 PC38, 0 PC34
//                      outByte - byte to write
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_write_byte(const byte card, const byte outByte)
{
#ifdef no_hardware
  return;
#else
  byte status;
  int data_reg, status_reg, icount; 
  if (card == PC38) {
    status_reg=pc38_status_reg;
    data_reg=pc38_data_reg;
  } else if (card == PC34) {
    status_reg=pc34_status_reg;
    data_reg=pc34_data_reg;
  } else if (card == GUIDERPC38) {
    status_reg=guiderpc38_status_reg;
    data_reg=guiderpc38_data_reg;
  }
 // wait until it is clear to transmit
  icount=0;
  while ((inportb(status_reg) & init_bit) == init_bit && icount<1000) {
    delay(1);
    icount++;
  }
  if (icount >=1000 ) {
    sprintf(tmpout,"pcx_write_byte error 1: ",card);
    writeline(tmpout,1);
  }
  icount=0;
  while ((inportb(status_reg) & tbe_bit) != tbe_bit && icount<1000) {
    delay(1);
    icount++;
  }
  if (icount >=1000 ) {
    sprintf(tmpout,"pcx_write_byte error 2: ",card);
    writeline(tmpout,1);
  }
  outportb(data_reg, outByte);

/*
  if (card == PC38) {
    while ((inportb(pc38_status_reg) & init_bit) == init_bit);
    while ((inportb(pc38_status_reg) & tbe_bit) != tbe_bit);
    while (((status=inportb(pc38_status_reg)) & init_bit) == init_bit) { 
      //sprintf(tmpout,"%x",status);
      //writeline(tmpout,1);
      delay(1);
      pcx_clear_read_register(card);
    }
    while (((status=inportb(pc38_status_reg)) & tbe_bit) != tbe_bit) {
      //sprintf(tmpout," %x",status);
      //writeline(tmpout,1);
      delay(1);
      pcx_clear_read_register(card);
    }
  } else if (card == PC34) {
    while ((inportb(pc34_status_reg) & init_bit) == init_bit);
    while ((inportb(pc34_status_reg) & tbe_bit) != tbe_bit);
  } else if (card == GUIDERPC38) {
    while ((inportb(guiderpc38_status_reg) & init_bit) == init_bit);
    while ((inportb(guiderpc38_status_reg) & tbe_bit) != tbe_bit);
  }
*/

  // introduce a 100 microsecond delay to accomodate some of our
  // computers.  Their busses aren't the greatest.  Use NOPs
/*
  for (int i = 0; i < 500; i++)
    asm
  {
    nop;
    nop;
    nop;
    nop;
    nop;
  }
*/
/*
  if (card == PC38)
    outportb(pc38_data_reg, outByte);
  else if (card == PC34)
    outportb(pc34_data_reg, outByte);
  else if (card == GUIDERPC38)
    outportb(guiderpc38_data_reg, outByte);
*/
#endif
}

//------------------------------------------------------------------
//      Name.......:    pcx_write_control
//
//  Purpose....:        send a byte to the PCX card control register
//
//      Input......:    card - 1 PC38, 0 PC34
//                      outByte - byte to write
//
//      Output.....:    none
//
//------------------------------------------------------------------
void pcx_write_control(const byte card, const byte outByte)
{
#ifdef no_hardware
  return;
#else
  if (card == PC38)
    outportb(pc38_control_reg, outByte);
  else if (card == PC34)
    outportb(pc34_control_reg, outByte);
  else if (card == GUIDERPC38)
    outportb(guiderpc38_control_reg, outByte);
#endif
}
/********************************* EOF **************************************/

/*
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
                        v = pcx_read_byte(card,0);

                while (v != 13)
                        v = pcx_read_byte(card,0);

                v = pcx_read_byte(card,0);
                while (!v)
                        v = pcx_read_byte(card,0);
                hexval[0] = v;

                v = pcx_read_byte(card,0);
                while (!v)
                        v = pcx_read_byte(card,0);
                hexval[1] = v;
                hexval[2] = 0;

                v = pcx_read_byte(card,0);
                while (v != 10)
                        v = pcx_read_byte(card,0);

                while (v != 13)
                        v = pcx_read_byte(card,0);
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
*/
unsigned pcx_read_inputs(const byte card)
        {
                #ifdef no_pc38
                return 0;
                #else

		int j, icount, nchar;
                byte v = 0;
                char hexval[7];
//	sprintf(tmpout,"pcx_read_inputs: %d",card);
//      writeline(tmpout,1);
        nchar=6;
        if ( card == PC34 ) nchar=2;

        // clear the read register and send the BX command to get the bit value
                pcx_clear_read_register(card);
                pcx_send_commands(card,"bx");

                // the number comes back as <lf><cr>##<lf><cr>
                icount=0;
                while (v != 10 && icount < 1000) {
                  v = pcx_read_byte(card,0);
                  icount++;
                  if (icount>1 || v !=10) {
                    sprintf(outbuf,"v1: d: %d c: %c",v,v);
                    if (icount<10) writeline(outbuf,1);
                  }
		}
	        if (icount == 1000) {
	          sprintf(tmpout,"error pcx_read_byte 1");
	          writeline(tmpout,1);
                  icount=0;
                }

                while (v != 13 && icount < 1000){
                  v = pcx_read_byte(card,0);
                  icount++;
		  //fprintf(stderr,"v2: %c ",v);
		}
	        if (icount == 1000) {
	          sprintf(tmpout,"error pcx_read_byte 2");
	          writeline(tmpout,1);
                  icount=0;
                }

	        for (j=0 ; j<nchar ; j++) {
                  v = pcx_read_byte(card,0);
                  while (!v && icount < 1000) {
		    //fprintf(stderr,"v%d: %c ",j,v);
                    v = pcx_read_byte(card,0);
                    icount++;
	          }
                  hexval[j] = v;
	          if (icount == 1000) {
	            sprintf(tmpout,"error pcx_read_byte 3 %d",j);
	            writeline(tmpout,1);
                    icount=0;
                  }
	        }
                hexval[nchar] = 0;

                v = pcx_read_byte(card,0);
                while (v != 10 && icount < 1000) {
		  //fprintf(stderr,"v5: %c ",v);
                  v = pcx_read_byte(card,0);
                  icount++;
		}
	        if (icount == 1000) {
	          sprintf(tmpout,"error pcx_read_byte 4");
	          writeline(tmpout,1);
                  icount=0;
                }

                while (v != 13 && icount < 1000) {
		  //fprintf(stderr,"v6: %c ",v);
                  v = pcx_read_byte(card,0);
                  icount++;
		}
	        if (icount == 1000) {
	          sprintf(tmpout,"error pcx_read_byte 5");
	          writeline(tmpout,1);
                  icount=0;
                }
		//fprintf(stderr,"\nhexval: %s\n",hexval);

//	sprintf(tmpout,"received pcx_read_inputs: %d %s",card,hexval);
//      writeline(tmpout,1);
                // convert hexval to an integer
                char hexdigits[] = "0123456789ABCDEFabcdef";
                unsigned rval = 0;

                for (byte i = 0; i < nchar; i++)
                   for (v = 0; v < 22; v++)
                      if (hexval[i] == hexdigits[v])
                        rval = (16 * rval) + ((v < 16) ? v : v - 6);

                return rval;
                #endif
        }

long return_position(const byte card, char axis, char *command,int show)
{
   #ifdef no_pc38
   return 0L;
   #else
   //char buffer[11];
   char buffer[81];
   char number[21];
   int idx, icount, err;
   byte rbyte;
   int maxcount=1000;

   idx=icount=err=0;
   if (show) {
     sprintf(tmpout,"pcx_return_position: %d %c %s %d\n",card,axis,command,err);
     writeline(tmpout,1);
   }

   sprintf(buffer, "a%c %s;", axis,command);
   pcx_send_commands(card,buffer);
   if (show) {
     writeline(buffer,1);
     sprintf(tmpout,"starting pc_read_byte");
     writeline(tmpout,1);
   }
   do {
     rbyte = pcx_read_byte(card,0);
     if (show && (icount<100 || icount%100 == 0)) {
       sprintf(tmpout,"%d %d ",rbyte,icount);
       writeline(tmpout,1);
     }
     icount++;
   } while (((rbyte == 0) || (rbyte == 10) || (rbyte == 13)) && (icount<maxcount));
   if (icount==maxcount) {
     delay(500);
     icount=0;
     do {
       rbyte = pcx_read_byte(card,0);
       icount++;
     } while (((rbyte == 0) || (rbyte == 10) || (rbyte == 13)) && (icount<maxcount));
   }
   if (icount==maxcount) {
     err=1;
   } else {
     number[idx++] = rbyte;
     icount=0;
     do {
         rbyte = pcx_read_byte(card,0);
         if (show && (idx<100 || idx%100 == 0)) {
           sprintf(tmpout,"%d   %d",rbyte, idx);
           writeline(tmpout,1);
         }
         if ((rbyte != 0) && (rbyte != 10) && (rbyte != 13))
           number[idx++] = rbyte;
         icount++;
     } while (rbyte != 10 && idx<20 && icount<maxcount);
     if (idx>=20 || icount>=maxcount) {
       err=2;
     } else {
       number[idx] = 0;

       pcx_clear_read_register(card);
       if (show) {
         sprintf(tmpout,"%c : %s %ld\n",axis,number,atol(number));
         writeline(tmpout,1);
       }
       return atol(number);
     }

   }
   if (err>0) {
     sprintf(tmpout,"pcx error: %d %c %s %d\n",card,axis,command,err);
     writeline(tmpout,1);
     pcx_clear_read_register(card);
     if (card==PC38) {
       tcs_set_deinit_telescope();
       tcs_telescope_stop(FALSE);
//       tcs_initialize();
     }
     return(0);
   }
   return(0);
   #endif
}



