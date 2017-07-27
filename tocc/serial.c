
  /*------------------------------------------------------------------*
                                 SERIAL.C

      * Compile this program with Test Stack Overflow OFF.

  *------------------------------------------------------------------*/

#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "serial.h"

#define VERSION 0x0101

#define FALSE           0
#define TRUE           (!FALSE)

#define NOERROR         0       /* No error               */
#define BUFOVFL         1       /* Buffer overflowed      */

#define ESC             0x1B    /* ASCII Escape character */
#define ASCII           0x007F  /* Mask ASCII characters  */
#define SBUFSIZ         0x4000  /* Serial buffer size     */

int            SError          = NOERROR;
int            portbase        = 0;
void           interrupt(*oldvects[2])();

static   char  ccbuf[SBUFSIZ];
unsigned int   startbuf        = 0;
unsigned int   endbuf          = 0;
static int escape=0;

#undef DEBUG

/* Handle communications interrupts and put them in ccbuf */
void   interrupt com_int(void)
{
#ifdef DEBUG
   fprintf(stderr,"got interrupt. Portbase: %d\n", portbase); 
#endif
   disable();
   if ((inportb(portbase + IIR) & RX_MASK) == RX_ID) {
      if (((endbuf + 1) & SBUFSIZ - 1) == startbuf) SError = BUFOVFL;

      ccbuf[endbuf] = inportb(portbase + RXR);
      if (ccbuf[endbuf] == 0x1b)
         escape=1;
      else
         endbuf++;
        
      endbuf &= SBUFSIZ - 1;
   }

   /* Signal end of hardware interrupt */
   outportb(ICR, EOI);
   enable();
}

/* Output a character to the serial port */
int    SerialOut(char x)
{
    long int           timeout = 0x0000FFFFL;

#undef HAVECTS
#ifdef HAVECTS
    outportb(portbase + MCR,  MC_INT | DTR | RTS);

    /* Wait for Clear To Send from modem */
    while ((inportb(portbase + MSR) & CTS) == 0)
        if (!(--timeout))
            return (-1);

    timeout = 0x0000FFFFL;

    /* Wait for transmitter to clear */
    while ((inportb(portbase + LSR) & XMTRDY) == 0)
        if (!(--timeout))
            return (-1);
#endif

    disable();
    outportb(portbase + TXR, x);
    enable();

    return (0);
}

/* Output a string to the serial port */
void   SendString(char *string)
{
    while (*string) {
       SerialOut(*string++);
       delay(1);
    }
    delay(10);
}

void SendShort(unsigned short *data, int nbytes)
{
    unsigned char *ptr;
    int i;

    ptr = data;
    for (i=0;i<nbytes;i++) {
fprintf(stderr,"sendshort: %d\n",i);
       SerialOut(*ptr++);
       delay(1);
    }
}

/* This routine returns the current value in the buffer */
int    getccb(void)
{
    int                res;

    if (endbuf == startbuf)
        return (-1);

    res = (int) ccbuf[startbuf++];
    startbuf %= SBUFSIZ;
    return (res);
}

/* Install our functions to handle communications */
void   setvects(void)
{
    oldvects[0] = getvect(0x0B);
    oldvects[1] = getvect(0x0C);
    setvect(0x0B, com_int);
    setvect(0x0C, com_int);
}

/* Uninstall our vectors before exiting the program */
void   resvects(void)
{
    setvect(0x0B, oldvects[0]);
    setvect(0x0C, oldvects[1]);
}

/* Turn on communications interrupts */
void   i_enable(int pnum)
{
    int                c;

    disable();
    c = inportb(portbase + MCR) | MC_INT;
    outportb(portbase + MCR, c);
    outportb(portbase + IER, RX_INT);
    c = inportb(IMR) & (pnum == COM1 ? IRQ4 : IRQ3);
    outportb(IMR, c);
    enable();
}

/* Turn off communications interrupts */
void   i_disable(void)
{
    int                c;

    disable();
    c = inportb(IMR) | ~IRQ3 | ~IRQ4;
    outportb(IMR, c);
    outportb(portbase + IER, 0);
    c = inportb(portbase + MCR) & ~MC_INT;
    outportb(portbase + MCR, c);
    enable();
}

/* Tell modem that we're ready to go */
void   comm_on(void)
{
    int                c, pnum;

    pnum = (portbase == COM1BASE ? COM1 : COM2);
    i_enable(pnum);
    c = inportb(portbase + MCR) | DTR | RTS;
    outportb(portbase + MCR, c);
}

/* Go off-line */
void   comm_off(void)
{
    i_disable();
    outportb(portbase + MCR, 0);
}

void   initserial(void)
{
    endbuf = startbuf = 0;
    setvects();
    comm_on();
}

void   closeserial(void)
{
    comm_off();
    resvects();
}

/* Set the port number to use */
int    SetPort(int Port)
{
    int                Offset, far *RS232_Addr;

    switch (Port)
    { /* Sort out the base address */
      case COM1 : Offset = 0x0000;
                  break;
      case COM2 : Offset = 0x0002;
                  break;
      default   : return (-1);
    }

    RS232_Addr = MK_FP(0x0040, Offset);  /* Find out where the port is. */
    if (*RS232_Addr == NULL) return (-1);/* If NULL then port not used. */
    portbase = *RS232_Addr;              /* Otherwise set portbase      */

    return (0);
}

/* This routine sets the speed; will accept funny baud rates. */
/* Setting the speed requires that the DLAB be set on.        */
int    SetSpeed(int Speed)
{
    char		c;
    int		divisor;

    if (Speed == 0)            /* Avoid divide by zero */
        return (-1);
    else
        divisor = (int) (115200L/Speed);

    if (portbase == 0)
        return (-1);

    disable();
    c = inportb(portbase + LCR);
    outportb(portbase + LCR, (c | 0x80)); /* Set DLAB */
    outportb(portbase + DLL, (divisor & 0x00FF));
    outportb(portbase + DLH, ((divisor >> 8) & 0x00FF));
    outportb(portbase + LCR, c);          /* Reset DLAB */
    enable();

    return (0);
}

/* Set other communications parameters */
int    SetOthers(int Parity, int Bits, int StopBit)
{
    int                setting;

    if (portbase == 0)					return (-1);
    if (Bits < 5 || Bits > 8)				return (-1);
    if (StopBit != 1 && StopBit != 2)			return (-1);
    if (Parity != NO_PARITY && Parity != ODD_PARITY && Parity != EVEN_PARITY)
							return (-1);

    setting  = Bits-5;
    setting |= ((StopBit == 1) ? 0x00 : 0x04);
    setting |= Parity;

    disable();
    outportb(portbase + LCR, setting);
    enable();

    return (0);
}

/* Set up the port */
int    SetSerial(int Port, int Speed, int Parity, int Bits, int StopBit)
{
    if (SetPort(Port))                    return (-1);
    if (SetSpeed(Speed))                  return (-1);
    if (SetOthers(Parity, Bits, StopBit)) return (-1);

    return (0);
}

/*  Control-Break interrupt handler */
int    c_break(void)
{
    i_disable();
    fprintf(stderr, "\nStill online.\n");

    return(0);
}

/* Get a string (terminated by \r or \n or \0) */
void GetString(char *str)
{
   int c;
   while( (c=getccb()) != -1 )
      *str++ = c;
   *str = '\0';
}

int GetChar(char *str)
{
   int c;
   if( (c=getccb()) != -1 ) {
      *str = c;
      return(0);
   } else
     return(-1);
}

int EscapeSeen(int clear)
{
   if (escape) {
      if (clear) escape=0;
      return(1);
      }
   else
      return(0);
}
