/****************************************************************************/
/*								*/
/*	Module:		pcx.h					*/
/*								*/
/*	Purpose:	low-level PC-34 and PC-38 motion control card controll				*/
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
/*								*/
/*	Author:		M. Donahue				*/
/*								*/
/****************************************************************************/
#ifndef _PCX_H
	#define _PCX_H

#include "mytype.h"

// definitions
#define pcx_x_axis	0x01
#define pcx_y_axis 	0x02
#define pcx_z_axis 	0x04
#define pcx_t_axis	0x08
#define pcx_u_axis	0x10
#define pcx_v_axis	0x20

#define PC34				0
#define PC38				1
#define GUIDERPC38			2
#define FILTERPC38			2

struct AXISPACK
{
  BOOL posdir;	// +/- movement
  BOOL done;	// done flag
  BOOL limit;	// limit status
  BOOL home;	// home status
};

// Actual common functions
extern byte pcx_check_status(const byte card);
extern void pcx_clear_read_register(const byte card);
extern void pcx_clear_done_flags(const byte card, const byte axes);
extern void pcx_disable_interrupts(const byte card);
extern BOOL pcx_done(const byte card, const byte axes);
extern void pcx_get_axis_status(const byte card, const char axis,struct AXISPACK &axisData);
extern byte pcx_read_byte(const byte card, int show);
extern byte pcx_read_status_reg(const byte card);
extern void pcx_send_commands(const byte card, const char *commands);
extern void pcx_write_byte(const byte card, const byte outByte);
extern void pcx_write_control(const byte card, const byte outByte);
unsigned pcx_read_inputs(const byte card);
extern long return_position(const byte card, char, char *,int);

// PC-34 functions
inline byte pc34_check_status()
	{ return pcx_check_status(PC34); }

inline void pc34_clear_read_register()
	{ pcx_clear_read_register(PC34); }

inline void pc34_clear_done_flags(const byte axes)
	{ pcx_clear_done_flags(PC34, axes); }

inline void pc34_disable_interrupts()
	{ pcx_disable_interrupts(PC34); }

inline BOOL pc34_done(const byte axes)
	{ return pcx_done(PC34, axes); }

inline void pc34_get_axis_status(const char axis, struct AXISPACK &axisData)
	{ pcx_get_axis_status(PC34, axis, axisData); }

inline byte pc34_read_byte(int show)
	{ return pcx_read_byte(PC34, show); }

inline byte pc34_read_status_reg()
	{ return pcx_read_status_reg(PC34); }

inline void pc34_send_commands(const char *commands)
	{ pcx_send_commands(PC34, commands); }

inline void pc34_write_byte(const byte outByte)
	{ pcx_write_byte(PC34, outByte); }

inline void pc34_write_control(const byte outByte)
	{ pcx_write_control(PC34, outByte); }

// PC-38 functions
inline byte pc38_check_status()
	{ return pcx_check_status(PC38); }

inline void pc38_clear_read_register()
	{ pcx_clear_read_register(PC38); }

inline void pc38_clear_done_flags(const byte axes)
	{ pcx_clear_done_flags(PC38, axes); }

inline void pc38_disable_interrupts()
	{ pcx_disable_interrupts(PC38); }

inline BOOL pc38_done(const byte axes)
	{ return pcx_done(PC38, axes); }

inline void pc38_get_axis_status(const char axis, struct AXISPACK &axisData)
	{ pcx_get_axis_status(PC38, axis, axisData); }

inline byte pc38_read_byte(int show)
	{ return pcx_read_byte(PC38, show); }

inline byte pc38_read_status_reg()
	{ return pcx_read_status_reg(PC38); }

inline void pc38_send_commands(const char *commands)
	{ pcx_send_commands(PC38, commands); }

inline void pc38_write_byte(const byte outByte)
	{ pcx_write_byte(PC38, outByte); }

inline void pc38_write_control(const byte outByte)
	{ pcx_write_control(PC38, outByte); }

#endif
/********************************* EOF **************************************/

