/****************************************************************************/
/*								*/
/*	Module:		tech80.cpp				*/
/*								*/
/*	Purpose:	routines to handle the dome encoder	*/
/*								*/
/****************************************************************************/
/*								*/
/*                    PROPERTY OF AUTOSCOPE CORPORATION         */
/*                        2637 Midpoint Dr., Suite D            */
/*                         Fort Collins, CO  80525              */
/*								*/
/*                            Copyright 1995                    */
/*              Unauthorized duplication or use is prohibited.	*/
/*								*/
/*	Author:		M. Donahue				*/
/*								*/
/****************************************************************************/
#include <dos.h>

#define cp4016

#ifdef tech80
#include "tech80.h"
#define t80_port	0x380
#endif
#ifdef cp4016
#include "cp4016.h"
#endif

//------------------------------------------------------------------
// 	Name.......:	init_dome_encoder
//
//  Purpose....:	prepare the 5312 quad encoder board for operation
//
//	Input......:	none
//
//	Output.....:	none
//
//------------------------------------------------------------------
void init_dome_encoder()
	{
		#ifdef tech80
		outportb(t80_port, 0x01);
		outportb(t80_port + 1, 0x35);
		outportb(t80_port + 1, 0x68);
		outportb(t80_port + 1, 0x80);
		outportb(t80_port + 1, 0xF1);
		#endif
	        #ifdef cp4016
                cp4016_init();
                cp4016_reset();
                #endif
	}

//------------------------------------------------------------------
// 	Name.......:	read_dome_encoder
//
//  Purpose....:	return the counter value
//
//	Input......:	none
//
//	Output.....:	counter value
//
//------------------------------------------------------------------
long read_dome_encoder()
	{
	unsigned long temp;
	temp = 0;

	#ifdef tech80
	outportb(t80_port, 1);
	outportb(t80_port + 1, 3);
	outportb(t80_port, 0);

	long i = inportb(t80_port + 1);
	long j = inportb(t80_port + 1);
	long k = inportb(t80_port + 1);
	unsigned long temp = i + (j * 0x100L) + (k * 0x10000L);
        if (temp > max_tech80_encoder_reading/2)
                  temp -= max_tech80_encoder_reading ;
	#endif
        #ifdef cp4016
        temp = cp4016_read_pos();
        #endif
	return temp;
	}

/********************************* EOF **************************************/

