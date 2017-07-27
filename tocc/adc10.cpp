/****************************************************************************
 *
 *               Copyright 1991-1995 Pico Technology Limited
 *
 * Product:      PICO ADC-10
 *
 * Module:       ADC-10A.C
 *
 * Author:       James L Luxford
 *               Pico Technology Limited
 *               Broadway House
 *               149-151 St Neots Road
 *               Hardwick
 *               Cambridge CB3 7QJ UK
 *               Tel. +44 (0) 1954 211716
 *               Fax. +44 (0) 1954 211880
 *               Email: 100073,2365@compuserve.com
 *
 * Description:  This module demonstrates how to drive the PICO ADC-10 from C.
 *               These routines will work at up to 6 khz: to use the ADC-10
 *               at full speed, see ADC-10B.C, which shows how to use
 *               the advanced driver.
 *
 * Routines:
 *  void adc10_open (int port);
 *      specify the printer port
 *
 *  int adc10_get_value(void);
 *      get a value from the adc-10
 *
 * Connections to ADC-10:
 *   Control: Output Address
 *
 *     Bit  7   6   5   4   3   2   1   0
 *     Pin  9   8   7   6   5   4   3   2
 *         PWR PWR PWR PWR PWR PWR *CS CLK
 *
 *   Input: Input Address
 *
 *     Bit  7   6   5   4   3   2   1   0
 *     Pin 10
 *        DATA
 *
 * History:
 *   ?????91 JLL Created
 *   08Sep93 MKG Remove power control routines
 *               Use port number, not address
 *   16Sep93 MKG Use #defines for everything
 *   26Sep93 MKG Add revision number
 *   28Sep93 MKG Change adc10_initialise to adc10_open
 *               Change adc10_get_value result from int to unsigned char
 *               Add clrscr
 *               Change CS comment
 *   05Oct93 MKG Change OUT to DATA
 *               Print out copyright notice
 *   09Feb95 MKG LPT1 default
 *   11Feb95 MKG Ask for port
 *
 * Revision Info: "file %n date %f revision %v"
 *                "file ADC-10A.C date 11-Feb-95,13:00:44 revision 7"
 *
 ****************************************************************************/

/* Change this to #undef MAIN if you want to link this routine
 *  into your own program
 */
#undef MAIN

#include <stdio.h>
#include <conio.h>
#include <dos.h>

/* This logic detects whether you are using Microsoft C.
 */
#ifdef _MSC_VER
#define outportb outp    /* Rename byte i/o routines for Microsoft */
#define inportb  inp
#endif

#define TRUE    1
#define FALSE   0

/* Output register values
 */

#define POWER   0xFC
#define OFF     0x00
#define CS      0x02
#define CLK     0x01

/* Input register values
 */
#define DATA    0x80

static int input_address;
static int output_address;

/****************************************************************************
 *
 * adc10_open
 *   sets 'output_address' and 'input_address' for the selected port
 *
 * accepts:
 *   port - port number (1 = LPT1, 2 = LPT2 et cetera)
 * returns:
 *   TRUE  - initialised successfully
 *   FALSE - invalid port
 *
 ****************************************************************************/

int adc10_open (int port)
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
    /* Lookup the port address in the BIOS printer table
     */
    output_address = printer_ports [port-1];
    if (output_address == 0)
      ok = FALSE;
    else
      {
      input_address = output_address + 1;

      /* Power on the ADC-10
       */
      outportb (output_address, POWER+CS);
      ok = TRUE;
      }
    }

  return ok;

  }


/****************************************************************************
 *
 * adc10_get_value - get a reading from the ADC-10
 *
 * This routine gets the result of the previous conversion from the ADC-10
 * and starts a new conversion
 *
 * accepts:
 *      nothing
 * returns:
 *      adc-10 value for previous conversion
 *        0   = 0V
 *        255 = 5V
 *
 ****************************************************************************/

int adc10_get_value (void)
  {
  int value = 0;
  int bit_value;
  int i;

  /* set *CS low - start conversion */
  outportb (output_address, POWER);

  for ( i = 1 ; i <= 8 ; i++ )
    {
    /* set CLOCK high */
    outportb (output_address, POWER+CLK);

    /* read bit */

    /* bit value = 0 or DATA */
    bit_value = inportb (input_address) & DATA;

    /* shift value left once */
    value *= 2;

    /* if input bit is zero,
     *  set the LSB
     */
    if (bit_value == 0)
      {
      value++;
      }

    /* set CLOCK low */
    outportb (output_address, POWER);
    }

  /* set *CS high - start conversion */
  outportb (output_address, POWER+CS);

  return (value);
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

  printf ("Simple ADC-10 C driver for DOS: Version 1.3\n");
  printf ("Copyright 1991-95 Pico Technology Limited\n");

  /* select printer port...
   */
   do
   {
       printf ("Printer port(1..4): ");
       port = getchar () - '0';
   } while ((port < 1) || (port > 4));
   adc10_open (port);

  printf ("Press any key to start displaying readings\n");
  printf ("Press a key again to stop\n");
  getch ();

  /* repeatedly print out adc values until a key is pressed
   */
  while (!kbhit ())
    {
    delay (500);
    value = adc10_get_value ();
    printf ("adc value = %d, voltage = %6.3f\n", value, value * 5.0 / 255);
    }

  }

#endif
