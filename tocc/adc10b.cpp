/****************************************************************************
 *
 *               Copyright 1995 Pico Technology Limited
 *
 * Product:      PICO ADC-10
 *
 * Module:       ADC-10B.C
 *
 * Author:       Mike Green
 *               Pico Technology Limited
 *               Broadway House
 *               149-151 St Neots Road
 *               Hardwick
 *               Cambridge CB3 7QJ UK
 *               Tel. +44 (0) 1954 211716
 *               Fax. +44 (0) 1954 211880
 *               Email: 100073,2365@compuserve.com
 *
 * Description:  This module shows how to use the advanced ADC-10 driver.
 *               It can be used under either DOS or windows.
 *
 * History:
 *   24Jan95 MKG Created
 *   11Feb95 MKG Add scaling, ask for port
 *
 * Revision Info: "file %n date %f revision %v"
 *                "file ADC-10B.C date 11-Feb-95,16:59:56 revision 6"
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#ifndef nohardware
#include "adc10.h"
#endif

#define TRUE    1
#define FALSE   0


//int buffer [1000];


/****************************************************************************
 *
 * main
 *
 *   simple test routine to show the use of the routines
 *
 ****************************************************************************/

void main (void)
  {
  long  expected_us;
  int   i;
  int   value;
  int   port;

  /* Say hello...
   */
  fprintf (stderr,"Advanced ADC-10 C driver for DOS: Version 1.3\n");
  fprintf (stderr,"Copyright 1995 Pico Technology Limited\n");

   port = 1;

#undef nohardware
#ifndef nohardware
   if (!adc10_driver_open (port))
     {
     fprintf (stderr,"Unable to open printer port\n");
     exit (99);
     }
#endif
  fprintf (stderr,"Press any key to start displaying readings\n");
  fprintf (stderr,"Press a key again to stop\n");
  getch ();

  /* repeatedly print out adc values until a key is pressed
   */
  while (!kbhit ())
    {
    delay (500);
#ifndef nohardware
    value = adc10_get_value ();
#endif
    printf ("adc value = %d, voltage = %6.3f\n", value, (value-128) * 5.0 / 128);
    }

  }
