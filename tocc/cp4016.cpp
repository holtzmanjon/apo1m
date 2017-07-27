#include <dos.h>
#include <math.h>
#include <stdio.h>
#include "cp4016.h"
#include "io.h"
#include "globals.h"
#include "tcs.h"

long cp4016_read_pos()
{
  unsigned char high, low;
  long motor_pos;
  int nrev;
  long encoder;

  outportb(0x3e0,0xf3);
  delay(10);
  high = inportb(0x3e2);
  delay(10);
  outportb(0x3e0,0xf7);
  delay(10);
  low = inportb(0x3e2);
  delay(10);

  outportb(0x3e0,0xcf);
  delay(10);
  high = inportb(0x3e2);
  delay(10);
  outportb(0x3e0,0xdf);
  delay(10);
  low = inportb(0x3e2);
  delay(10);

  outportb(0x3e0,0x3f);
  delay(10);
  high = inportb(0x3e2);
  delay(10);
  outportb(0x3e0,0x7f);
  delay(10);
  low = inportb(0x3e2);
  delay(10);

  outportb(0x3e0,0xfc);
  delay(10);
  high = inportb(0x3e2);
  delay(10);
  outportb(0x3e0,0xfd);
  delay(10);
  low = inportb(0x3e2);
  delay(10);
  //sprintf(outbuf,"1: high: %u low: %u  dec: %u\n",high,low,(high<<8)+low);
  //writeline(outbuf,1);
  encoder = (high<<8) + low;

#ifdef NOTDEF
  motor_pos = tcs_return_step_position('z');
  nrev = (long) floor (( motor_pos * 
         sysGlobal->z_encoder_encoder_steps_deg / sysGlobal->z_steps_degree -
         encoder ) / 65536. + 0.5);

  return (nrev*65536+encoder);
#endif
  return (encoder);
}

void cp4016_init()
{
  outportb(0x3e0,0xff);
  delay(10);
  outportb(0x3e1,0x0);
  delay(10);
  outportb(0x3e1,0xf);
  delay(10);
}

void cp4016_reset()
{
  outportb(0x3e1,0x0e);
  delay(10);
  outportb(0x3e1,0x0f);
  delay(10);
}

void cp4016_reset2()
{
  outportb(0x3e1,0xfe);
  delay(10);
  outportb(0x3e1,0xff);
  delay(10);
}
