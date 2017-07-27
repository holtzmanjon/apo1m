/****************************************************************************
 *
 *               Copyright 1991-1995 Pico Technology Limited
 *
 * Product:      PICO ADC-12 version 2
 *
 * Module:       ADC-12A.C
 *
 * Author:       Mike Green
 *               Pico Technology Limited
 *               Broadway House
 *               149-151 St Neots Road
 *               Hardwick
 *               Cambridge CB3 7QJ UK
 *               Tel. +44-1954-211716
 *               Fax. +44-1954-211880
 *               Email: 100073,2365@compuserve.com
 *
 * Description:  This module demonstrates how to drive the PICO ADC-12 from C.
 *               These routines will work at up to 6 khz: to use the ADC-12
 *               at full speed, see ADC-12B.C, which shows how to use
 *               the advanced ADC12 driver.
 *
 * Routines:
 *  void adc12_open (int port);
 *      specify the printer port
 *
 *  int adc12_get_value(void);
 *      get a value from the adc-12
 *
 * Connections to ADC-12 (V2):
 *   Control: Output Address
 *
 *     Bit  7   6   5   4   3   2   1   0
 *     Pin  9   8   7   6   5   4   3   2
 *         +5v +5v +5v +5v     +5v *CS CLK
 *         LOOP
 *
 *   Input: Input Address
 *
 *     Bit  7   6   5   4   3   2   1   0
 *     Pin 11  10
 *        DATA LOOP
 *
 * The LOOP between pins 9 and 10 can be used to detect V2
 *
 * History:
 *   15Oct94 MKG Created
 *   11feb95 MKG Ask for port
 *               Microsoft keywords
 *
 * Revision Info: "file %n date %f revision %v"
 *                "file ADC-12A.C date 11-Feb-95,18:52:06 revision 2"
 *
 ****************************************************************************/

/* Change this to #undef MAIN if you want to link this routine
 *  into your own program
 */
#undef MAIN


#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define TRUE    1
#define FALSE   0

/* This logic detects whether you are using Microsoft C.
 */
#ifdef _MSC_VER
#define outportb outp    /* Rename byte i/o routines for Microsoft */
#define inportb  inp
#endif

/* Output register values
 */

#define POWER   0xFC
#define CS      0x02
#define CLK     0x01

/* Input register values
 */
#define DATA    0x80

static int input_address;
static int output_address;

/****************************************************************************
 *
 * adc12_open
 *   sets 'output_address' and 'input_address' for the selected port
 *
 * accepts:
 *   port - port number (1 = LPT1, 2 = LPT2 et cetera)
 * returns:
 *   TRUE  - initialised successfully
 *   FALSE - invalid port
 *
 ****************************************************************************/

int adc12_open (int port)
  {
/* BIOS data area is segment 0x0040:
 * table of printer addresses is at offset 0x0008 within BIOS segment
 */
  unsigned int far * printer_ports = (unsigned int far *) 0x00400008L;
  int ok;


/* Port number must be 1 to 4
 */
  if ((port < 1) || (port > 4))
    ok = FALSE;
  else
    {
    output_address = printer_ports [port-1];
    if (output_address == 0)
      ok = FALSE;
    else
      {
      input_address = output_address + 1;

      /* Power on the ADC-12
       */
      outportb (output_address, POWER+CS);
      ok = TRUE;
      }
    }

  return ok;

  }


/****************************************************************************
 *
 * adc12_get_value - get a reading from the ADC-12
 *
 * This routine gets the result of the previous conversion from the ADC-12
 * and starts a new conversion
 *
 * This routine performs the following sequence:
 * Conversion sequence
 *      1.  Pull CS low (powers up the device)
 *      3.  Clock high (at least 300ns)
 *      4.  Clock low  (at least 400ns)
 *       5.  Read fist bit
 *
 *       6.  Clock high
 *       7.  Clock low
 *       8. Read second bit
 *       .
 *       .
 *       .
 *      xx. Read 12th bit
 *      xx. CS high (minimum of 500ns)
 *
 *       Minimum clock frequency is 100KHz
 *      Max clock frequency is 1MHz
 * accepts:
 *      nothing
 * returns:
 *      adc-12 value for previous conversion
 *      0    - 0V
 *      4095 - 5V
 *
 ****************************************************************************/


int adc12_get_value (void)
  {
  int   value;
  int   i;

  /* Put CS low
   */
  outportb (output_address, POWER);

  /* Clock twelve data bits from the ADC
   */
  value = 0;
  for ( i = 0; i < 12; i++)
    {
    value <<= 1;

    outportb (output_address, POWER+CLK);
    outportb (output_address, POWER);
    if (inportb (input_address) & DATA)
      {
      value++;
      }
    }


  /* Put CS high
   */
  outportb (output_address, POWER+CS);


  /* invert the result and mask off unused bits
   */
  value = ~value;
  value &= 0xFFF;

  return value;

}


#ifdef MAIN

/****************************************************************************
 *
 * main
 *
 *   simple test routine to show the use of the routines
 *
 ****************************************************************************/

void main (void)
  {
  int port;
  int value;

  /* Say hello...
   */
  printf ("Simple ADC-12 C driver for DOS: Version 1.1\n");
  printf ("Copyright 1991-5 Pico Technology Limited\n");

  /* select printer port...
   */
   do
   {
       printf ("Printer port (1..4): ");
       port = getchar () - '0';
   } while ((port < 1) || (port > 4));
   adc12_open (port);

  printf ("Press any key to start displaying readings\n");
  printf ("Press a key again to stop\n");
  getch ();

  /* repeatedly print out adc values until a key is pressed
   */
  while (!kbhit ())
    {
    delay (500);
    value = adc12_get_value ();
    printf ("adc value = %d, voltage = %6.3f\n", value, value * 5.0 / 4095);
    }

  }

#endif
