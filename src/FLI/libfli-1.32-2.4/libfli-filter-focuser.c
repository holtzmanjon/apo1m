/*

  Copyright (c) 2002 Finger Lakes Instrumentation (FLI), L.L.C.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

        Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above
        copyright notice, this list of conditions and the following
        disclaimer in the documentation and/or other materials
        provided with the distribution.

        Neither the name of Finger Lakes Instrumentation (FLI), LLC
        nor the names of its contributors may be used to endorse or
        promote products derived from this software without specific
        prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  ======================================================================

  Finger Lakes Instrumentation, L.L.C. (FLI)
  web: http://www.fli-cam.com
  email: support@fli-cam.com

*/

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/param.h>
#include <netinet/in.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>

#include "libfli-libfli.h"
#include "libfli-mem.h"
#include "libfli-debug.h"
#include "libfli-filter-focuser.h"

extern double dconvert(void *buf); /* From libfli-camera-usb.c */

/*
Array of filterwheel info
	Pos = # of filters
	Off = Offset of 0 filter from magnetic stop,
	X - y = number of steps from filter x to filter y
*/
static const wheeldata_t wheeldata[] =
{
  /* POS OFF   0-1 1-2 2-3 3-4 4-5 5-6 6-7 7-8 8-9 9-A A-B B-C C-D D-E F-F F-0 */
  { 3, 48, { 80, 80, 80, 80,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0} }, /* Index 0 */
  { 5,  0, { 48, 48, 48, 48, 48,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0} }, /* Index 1 */
  { 7, 14, { 34, 34, 35, 34, 34, 35, 35,  0,  0,  0,  0,  0,  0,  0,  0,  0} }, /* Index 2 */
  { 8, 18, { 30, 30, 30, 30, 30, 30, 30, 30,  0,  0,  0,  0,  0,  0,  0,  0} }, /* Index 3 */
  {10,  0, { 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,  0,  0,  0,  0,  0,  0} }, /* Index 4 */
  {12,  6, { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,  0,  0,  0,  0} }, /* Index 5 */
  {15,  0, { 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48} }, /* Index 6 */
	{ 7, 14, { 52, 52, 52, 52, 52, 52, 52, 52,  0,  0,  0,  0,  0,  0,  0,  0} }, /* Index 7 */
};

#define FLI_BLOCK (1)
#define FLI_NON_BLOCK (0)

static long fli_stepmotor(flidev_t dev, long steps, long block);
static long fli_getsteppos(flidev_t dev, long *pos);
static long fli_setfilterpos(flidev_t dev, long pos);
static long fli_getstepsremaining(flidev_t dev, long *pos);
static long fli_focuser_getfocuserextent(flidev_t dev, long *extent);
static long fli_focuser_readtemperature(flidev_t dev, flichannel_t channel, double *temperature);

long fli_filter_focuser_probe(flidev_t dev)
{
  int err = 0;
  long rlen, wlen;
  unsigned short buf[16];

	CHKDEVICE(dev);

  DEVICE->io_timeout = 200;

  wlen = 2;
  rlen = 2;
  buf[0] = htons(0x8000);
  IO(dev, buf, &wlen, &rlen);
  if (ntohs(buf[0]) != 0x8000)
  {
    debug(FLIDEBUG_WARN, "Invalid echo, no FLI serial device found.");
    err = -ENODEV;
  }

	return err;
}

long fli_filter_focuser_open(flidev_t dev)
{
  int err = 0;
  unsigned long ndev;
  long rlen, wlen;
  unsigned short buf[16];
  flifilterdata_t *fdata = NULL;

  CHKDEVICE(dev);

  DEVICE->io_timeout = 200;

  wlen = 2;
  rlen = 2;
  buf[0] = htons(0x8000);
  IO(dev, buf, &wlen, &rlen);
  if (ntohs(buf[0]) != 0x8000)
  {
    debug(FLIDEBUG_WARN, "Invalid echo, device not recognized, got %04x instead of %04x.", ntohs(buf[0]), 0x8000);
    err = -ENODEV;
    goto done;
  }

  wlen = 2;
  rlen = 2;
  buf[0] = htons(0x8001);
  IO(dev, buf, &wlen, &rlen);
  DEVICE->devinfo.fwrev = ntohs(buf[0]);
  if ((DEVICE->devinfo.fwrev & 0xff00) != 0x8000)
  {
    debug(FLIDEBUG_WARN, "Invalid echo, device not recognized.");
    err = -ENODEV;
    goto done;
  }

  if ((DEVICE->device_data = xmalloc(sizeof(flifilterdata_t))) == NULL)
  {
    err = -ENOMEM;
    goto done;
  }

	fdata = DEVICE->device_data;
  fdata->tableindex = -1;
  fdata->stepspersec = 100;
  fdata->currentslot = -1;

  if (DEVICE->devinfo.fwrev == 0x8001)  /* Old level of firmware */
  {
    if (DEVICE->devinfo.type != FLIDEVICE_FILTERWHEEL)
    {
			debug(FLIDEBUG_INFO, "Device detected is not filterwheel, old firmware?");
      err = -ENODEV;
      goto done;
    }

    debug(FLIDEBUG_INFO, "Device is old fashioned filter wheel.");
    fdata->tableindex = 1;

    // FIX: should model info be set first?
    return 0;
  }

  debug(FLIDEBUG_INFO, "New version of hardware found.");
  wlen = 2;
  rlen = 2;
  buf[0] = htons(0x8002);
  IO(dev, buf, &wlen, &rlen);
  ndev = ntohs(buf[0]);

  if ((ndev & 0xff00) != 0x8000)
  {
    err = -ENODEV;
    goto done;
  }

	/* ndev is either jumper settings or FW return code */
  ndev &= 0x00ff;
  switch (ndev)
  {
		case 0x00:
			if (DEVICE->devinfo.type != FLIDEVICE_FILTERWHEEL)
			{
				err = -ENODEV;
				goto done;
			}
			fdata->tableindex = 1;
			break;

		case 0x01:
			if (DEVICE->devinfo.type != FLIDEVICE_FILTERWHEEL)
			{
				err = -ENODEV;
				goto done;
			}
			fdata->tableindex = 0;
			break;

		case 0x02:
			if (DEVICE->devinfo.type != FLIDEVICE_FILTERWHEEL)
			{
				err = -ENODEV;
				goto done;
			}
			fdata->tableindex = 2;
			break;

		case 0x03:
			if (DEVICE->devinfo.type != FLIDEVICE_FILTERWHEEL)
			{
				err = -ENODEV;
				goto done;
			}
			fdata->tableindex = 3;
			break;

		case 0x04:
			if (DEVICE->devinfo.type != FLIDEVICE_FILTERWHEEL)
			{
				err = -ENODEV;
				goto done;
			}
			fdata->tableindex = 6;
			fdata->stepspersec= 16; // 1 /.06
			break;

		case 0x05:
			if (DEVICE->devinfo.type != FLIDEVICE_FILTERWHEEL)
			{
				err = -ENODEV;
				goto done;
			}
			fdata->tableindex = 5;
			fdata->stepspersec= 16; //   1/.06
	    break;

		case 0x06:
			if (DEVICE->devinfo.type != FLIDEVICE_FILTERWHEEL)
			{
				err = -ENODEV;
				goto done;
			}
			fdata->tableindex = 4;
			fdata->stepspersec= 16; //   1/.06
			break;

		case 0x07:
			if (DEVICE->devinfo.type != FLIDEVICE_FOCUSER)
			{
				err = -ENODEV;
				goto done;
			}
			break;

		case 0x08:
			if (DEVICE->devinfo.type != FLIDEVICE_FILTERWHEEL)
			{
				err = -ENODEV;
				goto done;
			}
			fdata->tableindex = 7;
			fdata->stepspersec= 50;
			break;

		default:
		  debug(FLIDEBUG_FAIL, "Unknown device attached.");
			err = -ENODEV;
			goto done;
  }

	/* Now get the model name, either construct it or get it from device. */
	if (err == 0)
	{
		if (DEVICE->devinfo.type == FLIDEVICE_FILTERWHEEL)
		{
			if ((DEVICE->devinfo.fwrev & 0x00ff) <= 0x30)
			{
				if (xasprintf(&DEVICE->devinfo.model, "Filter Wheel (%ld position)",
						wheeldata[fdata->tableindex].n_pos) < 0)
				{
					debug(FLIDEBUG_WARN, "Could not allocate memory for model information.");
				}
			}

			if ((DEVICE->devinfo.fwrev & 0x00ff) >= 0x31)
			{
				if ((DEVICE->devinfo.model = (char *) xmalloc(33)) == NULL)
				{
					debug(FLIDEBUG_WARN, "Could not allocate memory for model information.");
				}
				else
				{
					memset(DEVICE->devinfo.model, 0x00, 33);
					wlen = 2;
					rlen = 32;
					DEVICE->devinfo.model[0] = 0x80;
					DEVICE->devinfo.model[1] = 0x03;
					IO(dev, DEVICE->devinfo.model, &wlen, &rlen);
				}
			}
		}

		if (DEVICE->devinfo.type == FLIDEVICE_FOCUSER)
		{
			if (xasprintf(&DEVICE->devinfo.model, "FLI Focuser",
					wheeldata[fdata->tableindex].n_pos) < 0)
			{
				debug(FLIDEBUG_WARN, "Could not allocate memory for model information.");
			}
		}
	}

 done:
  if (err)
  {
    if (DEVICE->devinfo.model != NULL)
    {
      xfree(DEVICE->devinfo.model);
      DEVICE->devinfo.model = NULL;
    }

    if (DEVICE->device_data != NULL)
    {
      xfree(DEVICE->device_data);
      DEVICE->device_data = NULL;
    }

    return err;
  }

  debug(FLIDEBUG_INFO, "Found '%s'", DEVICE->devinfo.model);
  return 0;
}

long fli_filter_focuser_close(flidev_t dev)
{
  CHKDEVICE(dev);

  if (DEVICE->devinfo.model != NULL)
  {
    xfree(DEVICE->devinfo.model);
    DEVICE->devinfo.model = NULL;
  }

  if (DEVICE->device_data != NULL)
  {
    xfree(DEVICE->device_data);
    DEVICE->device_data = NULL;
  }

  return 0;
}

long fli_filter_command(flidev_t dev, int cmd, int argc, ...)
{
  flifilterdata_t *fdata;
  long r;
  va_list ap;

  va_start(ap, argc);
  CHKDEVICE(dev);
  fdata = DEVICE->device_data;

  switch (cmd)
  {
		case FLI_SET_FILTER_POS:
			if (argc != 1)
				r = -EINVAL;
			else
			{
				long pos;

				pos = *va_arg(ap, long *);
				r = fli_setfilterpos(dev, pos);
			}
			break;

		case FLI_GET_FILTER_POS:
			if (argc != 1)
				r = -EINVAL;
			else
			{
				long *cslot;

				cslot = va_arg(ap, long *);
				*cslot = fdata->currentslot;
				r = 0;
			}
			break;

		case FLI_GET_FILTER_COUNT:
			if (argc != 1)
				r = -EINVAL;
			else
			{
				long *nslots;

				nslots = va_arg(ap, long *);
				*nslots = wheeldata[fdata->tableindex].n_pos;
				r = 0;
			}
			break;

		case FLI_STEP_MOTOR:
			if (argc != 1)
				r = -EINVAL;
			else
			{
				long *steps;

				steps = va_arg(ap, long *);
				r = fli_stepmotor(dev, *steps, FLI_BLOCK);
			}
			break;

		case FLI_STEP_MOTOR_ASYNC:
			if (argc != 1)
				r = -EINVAL;
			else
			{
				long *steps;

				steps = va_arg(ap, long *);
				r = fli_stepmotor(dev, *steps, FLI_NON_BLOCK);
			}
			break;

  case FLI_GET_STEPPER_POS:
    if (argc != 1)
      r = -EINVAL;
    else
    {
      long *pos;

      pos = va_arg(ap, long *);
      r = fli_getsteppos(dev, pos);
    }
    break;

  case FLI_GET_STEPS_REMAINING:
    if (argc != 1)
      r = -EINVAL;
    else
    {
      long *pos;

      pos = va_arg(ap, long *);
      r = fli_getstepsremaining(dev, pos);
    }
    break;

  default:
    r = -EINVAL;
  }

  va_end(ap);

  return r;
}

long fli_focuser_command(flidev_t dev, int cmd, int argc, ...)
{
  long r;
  va_list ap;

  va_start(ap, argc);
  CHKDEVICE(dev);

  switch (cmd)
  {
  case FLI_STEP_MOTOR:
    if (argc != 1)
      r = -EINVAL;
    else
    {
      long *steps;

      steps = va_arg(ap, long *);
      r = fli_stepmotor(dev, *steps, FLI_BLOCK);
    }
    break;

		case FLI_STEP_MOTOR_ASYNC:
			if (argc != 1)
				r = -EINVAL;
			else
			{
				long *steps;

				steps = va_arg(ap, long *);
				r = fli_stepmotor(dev, *steps, FLI_NON_BLOCK);
			}
			break;

  case FLI_GET_STEPPER_POS:
    if (argc != 1)
      r = -EINVAL;
    else
    {
      long *pos;

      pos = va_arg(ap, long *);
      r = fli_getsteppos(dev, pos);
    }
    break;

  case FLI_GET_STEPS_REMAINING:
    if (argc != 1)
      r = -EINVAL;
    else
    {
      long *pos;

      pos = va_arg(ap, long *);
      r = fli_getstepsremaining(dev, pos);
    }
    break;

  case FLI_GET_FOCUSER_EXTENT:
    if (argc != 1)
      r = -EINVAL;
    else
    {
      long *extent;

      extent = va_arg(ap, long *);
      r = fli_focuser_getfocuserextent(dev, extent);
    }
    break;

  case FLI_HOME_FOCUSER:
    if (argc != 0)
      r = -EINVAL;
    else
      r =  fli_setfilterpos(dev, FLI_FILTERPOSITION_HOME);
    break;

  case FLI_READ_TEMPERATURE:
    if (argc != 2)
      r = -EINVAL;
    else
    {
      double *temperature;
			flichannel_t channel;

      channel = va_arg(ap, flichannel_t);
			temperature = va_arg(ap, double *);
      r = fli_focuser_readtemperature(dev, channel, temperature);
    }
    break;

  default:
    r = -EINVAL;
  }

  va_end(ap);

  return r;
}

static long fli_stepmotor(flidev_t dev, long steps, long block)
{
  flifilterdata_t *fdata;
  long dir, timeout, move, stepsleft;
  long rlen, wlen;
  unsigned short buf[16];
  clock_t begin;

  fdata = DEVICE->device_data;

/* Support HALT operation when steps == 0 */
	if (steps == 0)
	{
    rlen = 2;
    wlen = 2;
    buf[0] = htons((unsigned short) 0xa000);
    IO(dev, buf, &wlen, &rlen);
    if ((ntohs(buf[0]) & 0xf000) != 0xa000)
    {
			debug(FLIDEBUG_WARN, "Invalid echo.");
			return -EIO;
    }
		return 0;
	}

  dir = steps;
  steps = abs(steps);
  while (steps > 0)
  {
    if (steps > 4095)
      move = 4095;
    else
      move = steps;

    debug(FLIDEBUG_INFO, "Stepping %d steps.", move);

    steps -= move;
    timeout = (move / fdata->stepspersec) + 2;

    rlen = 2;
    wlen = 2;
    if (dir < 0)
    {
      buf[0] = htons((unsigned short) (0xa000 | (unsigned short) move));
      IO(dev, buf, &wlen, &rlen);
      if ((ntohs(buf[0]) & 0xf000) != 0xa000)
      {
				debug(FLIDEBUG_WARN, "Invalid echo.");
				return -EIO;
      }
    }
    else
    {
      buf[0] = htons((unsigned short) (0x9000 | (unsigned short) move));
      IO(dev, buf, &wlen, &rlen);
      if ((ntohs(buf[0]) & 0xf000) != 0x9000)
      {
				debug(FLIDEBUG_WARN, "Invalid echo.");
				return -EIO;
      }
    }

    begin = clock();
    stepsleft = 0;
    while ( (stepsleft != 0x7000) && (block != 0) )
    {
#ifdef WIN32
			Sleep(100);
#else
			usleep(100000);
#endif
			buf[0] = htons(0x7000);
      IO(dev, buf, &wlen, &rlen);
      stepsleft = ntohs(buf[0]);

      if (((clock() - begin) / CLOCKS_PER_SEC) > timeout)
      {
				debug(FLIDEBUG_WARN, "A device timeout has occurred.");
				return -EIO;
      }
    }
  }

  return 0;
}

static long fli_getsteppos(flidev_t dev, long *pos)
{
  long poslow, poshigh;
  long rlen, wlen;
  unsigned short buf[16];

  rlen = 2; wlen = 2;
  buf[0] = htons(0x6000);
  IO(dev, buf, &wlen, &rlen);
  poslow = ntohs(buf[0]);
  if ((poslow & 0xf000) != 0x6000)
    return -EIO;

  buf[0] = htons(0x6001);
  IO(dev, buf, &wlen, &rlen);
  poshigh = ntohs(buf[0]);
  if ((poshigh & 0xf000) != 0x6000)
    return -EIO;

  if ((poshigh & 0x0080) > 0)
  {
    *pos = ((~poslow) & 0xff) + 1;
    *pos += (256 * ((~poshigh) & 0xff));
    *pos = -(*pos);
  }
  else
  {
    *pos = (poslow & 0xff) + 256 * (poshigh & 0xff);
  }
  return 0;
}

static long fli_getstepsremaining(flidev_t dev, long *pos)
{
  long rlen = 2, wlen = 2;
  unsigned short buf[16];

	buf[0] = htons(0x7000);
  IO(dev, buf, &wlen, &rlen);
  *pos = ntohs(buf[0]) & 0x0fff;

	return 0;
}

static long fli_setfilterpos(flidev_t dev, long pos)
{
  flifilterdata_t *fdata;
  long rlen, wlen;
  unsigned short buf[16];
  long move, i, steps;

  fdata = DEVICE->device_data;

  if (pos == FLI_FILTERPOSITION_HOME)
    fdata->currentslot = FLI_FILTERPOSITION_HOME;

  if (fdata->currentslot < 0)
  {
    debug(FLIDEBUG_INFO, "Home filter wheel/focuser.");
		//set the timeout
    DEVICE->io_timeout = (DEVICE->devinfo.type == FLIDEVICE_FILTERWHEEL ? 5000 : 30000);
		//10,12,15 pos filterwheel  needs a longer timeout t

		switch (wheeldata[fdata->tableindex].n_pos)
		{
			case 12:
			case 10:
				DEVICE->io_timeout = 120000;
				break;

			case 15:
				DEVICE->io_timeout = 200000;
				break;

			default:
				break;
		}

    wlen = 2;
    rlen = 2;
    buf[0] = htons(0xf000);
    IO(dev, buf, &wlen, &rlen);
    if (ntohs(buf[0]) != 0xf000)
      return -EIO;

		/* Reduce overall timeout to speed operations with serial port */
    DEVICE->io_timeout = 200;

		/* This is required to prevent offsetting the focuser */
		if (DEVICE->devinfo.type != FLIDEVICE_FOCUSER)
		{
			debug(FLIDEBUG_INFO, "Moving %d steps to home position.",
				wheeldata[fdata->tableindex].n_offset);

	    COMMAND(fli_stepmotor(dev, - (wheeldata[fdata->tableindex].n_offset), FLI_BLOCK));
		  fdata->currentslot = 0;
		}
  }

  if (pos == FLI_FILTERPOSITION_HOME)
    return 0;

  if (pos >= wheeldata[fdata->tableindex].n_pos)
  {
    debug(FLIDEBUG_WARN, "Requested slot (%d) exceeds number of slots.", pos);
    return -EINVAL;
  }

  if (pos == fdata->currentslot)
    return 0;

  move = pos - fdata->currentslot;

  if (move < 0)
    move += wheeldata[fdata->tableindex].n_pos;

  steps = 0;
  for (i=0; i < move; i++)
    steps += wheeldata[fdata->tableindex].n_steps[i % wheeldata[fdata->tableindex].n_pos];

  debug(FLIDEBUG_INFO, "Move filter wheel %d steps.", steps);

  COMMAND(fli_stepmotor(dev, - (steps), FLI_BLOCK));
  fdata->currentslot = pos;

  return 0;
}
long fli_focuser_getfocuserextent(flidev_t dev, long *extent)
{
	if ((DEVICE->devinfo.fwrev & 0x00ff) < 0x30)
	{
		*extent = 2100;
	}
	else
	{
		*extent = 7000;
	}

	return 0;
}

long fli_focuser_readtemperature(flidev_t dev, flichannel_t channel, double *temperature)
{
	long rlen, wlen;
	short buf[64];
	short b;
	int i;

	if ((DEVICE->devinfo.fwrev & 0x00ff) < 0x30)
	{
    debug(FLIDEBUG_WARN, "This device does not support temperature reading.");
		return -EINVAL;
	}

	if ((DEVICE->devinfo.fwrev & 0x00ff) == 0x30)
	{
		if (channel > 0)
		{
			debug(FLIDEBUG_WARN, "This device supports only one channel.");
			return -EINVAL;
		}

		wlen = 2;
		rlen = 2;
		buf[0] = htons(0x1000 | (unsigned short) channel);
		IO(dev, buf, &wlen, &rlen);

		b = ntohs(buf[0]);
		*temperature = (double) b / 256.0;
	}

	if ((DEVICE->devinfo.fwrev & 0x00ff) > 0x30)
	{
		if (channel > 1)
		{
			debug(FLIDEBUG_WARN, "This device supports only two channels.");
			return -EINVAL;
		}

		/* Ok, some constants are sent back with each temperature reading */
		wlen = 2;
		rlen = 2 + 4 * 7;
		buf[0] = htons(0x1000 | (unsigned short) channel);
		IO(dev, buf, &wlen, &rlen);

		b = ntohs(buf[0]);
		*temperature = 0.0;
		for (i = 0; i < 7; i++)
		{
/*			debug(FLIDEBUG_INFO, "A[%d] = %f", i, dconvert(((char *) buf) + (2 + i * 4))); */
			*temperature += dconvert(((char *) buf) + (2 + i * 4)) * pow((double) b, (double) i);
		}

		if (*temperature < (-45.0))
		{
			debug(FLIDEBUG_WARN, "External sensor not plugged in.");
			return -EINVAL;
		}
	}

	return 0;
}
