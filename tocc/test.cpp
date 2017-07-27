#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include "mytype.h"
#include "pcx.h"

#define update

double x_encoder_steps_deg = 62445.063889;
double x_steps_deg = 74934.;
double x_encoder_steps_rev = 512000.;
double x_motor_steps_rev = 614400.;
double x_max_velocity = 248308.;

double y_steps_deg = 62077.1556;
double y_encoder_steps_deg = 62577.7;
double y_encoder_steps_rev = 512000.;
double y_motor_steps_rev = 507904.;
double y_max_velocity = 248308.;

double z_steps_deg = 28796.630556;
double z_encoder_steps_deg = 62577.7;
double z_encoder_steps_rev = 512000.;
double z_motor_steps_rev = 507904.;
double z_max_velocity = 114777.;

double ref_az = 305.197136;
double ref_alt = 79.6788;

long tcs_return_encoder_position(char axis);
long tcs_return_step_position(char axis);

void mygettime(struct date, struct mytime *);
char outbuf[300];

#define NPTS 1200

main()
{
  char command[80], file[80];
  unsigned int counter = 0;
  int i, iaxis, ispeed;
  double sign;
  long enc, motor;
  struct date d;
  struct mytime t;
  double t0, tnow, rate, rate0, dt, e0, targ, curr, curr_motor, now, dr;
  double last_rate, dtr;
  double steps_deg, encoder_steps_deg, encoder_steps_rev;
  double motor_steps_rev, max_velocity;
#ifdef NOWRITE
  double out[NPTS][9];
#endif
  FILE *debug_file;
  char axis, eaxis;
  char id[64];

  printf("Enter string identifier for this frequency/pointing/etc: ");
  scanf("%s",&id);

  printf("Enter sign of motion (1 or -1): ");
  scanf("%lf",&sign);

#ifdef update
  printf("Enter update dt in seconds (<=0 for no feedback): ");
  scanf("%lf",&dt);

  printf("Enter maximum percent change in rate(e.g., 0.1 to 0.5): ");
  scanf("%lf",&dr);
#endif

// Loop over four motor speeds in each axis
  for (iaxis=0;iaxis<2;iaxis++) {
    if (iaxis==0) {
      axis = 'x';
      eaxis = 'x';
      encoder_steps_deg = x_encoder_steps_deg;
      steps_deg = x_steps_deg;
      encoder_steps_rev = x_encoder_steps_rev;
      motor_steps_rev = x_motor_steps_rev;
      max_velocity = x_max_velocity;
    }
    else if (iaxis==1) {
#ifdef zswap
      axis = 'z';
#else
      axis = 'y';
#endif
      eaxis = 'y';
      encoder_steps_deg = y_encoder_steps_deg;
      steps_deg = y_steps_deg;
      encoder_steps_rev = y_encoder_steps_rev;
      motor_steps_rev = y_motor_steps_rev;
      max_velocity = y_max_velocity;
    }
    else if (iaxis==2) {
      axis = 'z';
      eaxis = 'y';
      encoder_steps_deg = z_encoder_steps_deg;
      steps_deg = z_steps_deg;
      encoder_steps_rev = z_encoder_steps_rev;
      motor_steps_rev = z_motor_steps_rev;
      max_velocity = z_max_velocity;
    }

    for (ispeed=1;ispeed<=4;ispeed++) {
#ifdef no_hardware
      sprintf(file,"c:\\tocc\\test\\test%s%c%d.dat",id,axis,ispeed);
#else
      sprintf(file,"d:\\tocc\\test\\test%s%c%d.dat",id,axis,ispeed);
#endif
      fprintf(stderr,"%s\n",file);
      debug_file = fopen(file,"w");

      rate0 = ispeed * 0.0042 / 4;
      if (iaxis == 0) rate0 *= 3;
      rate0 *= sign;
printf("rate0: %f\n",rate0);

// initialize encoders and motors
      enc = 0;
     
      sprintf(command,"a%c er%ld,%ld lp%ld;",
                   axis,
                   encoder_steps_rev,
                   motor_steps_rev,
                   enc);
      #ifndef no_hardware
      pc38_send_commands(command);
      #endif
      sprintf(command,"a%c er%ld,%ld lp%ld;",
                   eaxis,
                   encoder_steps_rev,
                   motor_steps_rev,
                   enc);
      #ifndef no_hardware
      pc38_send_commands(command);
      #endif

      ref_az = ref_alt = 0.;

// Get the current time (in sec)
      mygettime(&d,&t);
      t0 = t.ti_hour*3600.;
      t0 += t.ti_min*60.;
      t0 += t.ti_sec;
      t0 += t.ti_hund/100.;

// Get the current position (in degrees)
      e0 = tcs_return_encoder_position(eaxis) / encoder_steps_deg;

// Start the motor going at a constant rate in (deg/s)
      sprintf(command,"a%c hf; ac 62077; jf%.4f ",axis,rate0*steps_deg);
      #ifndef no_hardware
      pc38_send_commands(command);
      #endif

      dtr = dt*rate0;

// Now loop and query motors and encoders
      for (i=0;i<NPTS;i++) {

        // get current time
        mygettime(&d,&t);
        // encoder and motor positions
        motor = tcs_return_step_position(axis);
        enc = tcs_return_encoder_position(eaxis);
// if (iaxis==1) enc = -1 * enc;

        tnow = t.ti_hour*3600.;
        tnow += t.ti_min*60.;
        tnow += t.ti_sec;
        tnow += t.ti_hund/100.;

        // desired position now
        now = e0 + (tnow-t0)*rate0;
        // desired position in dt seconds in degrees
        targ = now + dtr;
        // current position in degrees
        curr = enc / encoder_steps_deg ;
        curr_motor = motor / steps_deg ;

#ifdef update
        if (dt>0) {
          // compute new rate in motor steps/s
          last_rate = rate;
          rate = (targ-curr) / dt;
          rate = min(max(rate,(double)((1.-dr)*rate0)),
                             (double)((1.+dr)*rate0));
          rate *= steps_deg;
          rate = min(max(rate,(double)(-0.1*max_velocity)),
                             (double)(0.1*max_velocity));
          // send the new rate
          if (rate < 1000)
            sprintf(command,"a%c jf%.4f ",axis,rate);
          else
            sprintf(command,"a%c jg%ld ",axis,(long)rate);
          #ifndef no_hardware
          pc38_send_commands(command);
          #endif
        }
#endif

#ifdef NOWRITE
        out[i][1] = tnow;
        out[i][2] = enc;
        out[i][3] = motor;
        out[i][4] = (curr-now);
        out[i][5] = (curr_motor-now);
        out[i][6] = curr;
        out[i][7] = now;
        out[i][8] = targ;
        out[i][9] = rate0;
#else
        fprintf(stderr,
         "%8.3lf %u %ld %ld %8.3lf %8.3lf %8.6lf %8.6lf %8.6lf %8.6lf %8.6lf\n",
         tnow,counter,enc,motor,(curr-now)*3600.,(curr_motor-now)*3600.,
         curr,now,targ,rate0,rate/steps_deg);
        fprintf(debug_file,
         "%8.3lf %u %ld %ld %8.3lf %8.3lf %8.6lf %8.6lf %8.6lf %8.6lf %8.6lf\n",
         tnow,counter,enc,motor,(curr-now)*3600.,(curr_motor-now)*3600.,
         curr,now,targ,rate0,rate/steps_deg);
#endif

      }
      sprintf(command,"a%c st;",axis);
      #ifndef no_hardware
      pc38_send_commands(command);
      #endif
#ifdef NOWRITE
      for (i=0;i<NPTS;i++) {
        fprintf(debug_file,
          "%8.3lf %ld %ld %8.3lf %8.3lf %8.6lf %8.6lf %8.6lf %8.6lf\n",
          out[i][1],(long)out[i][2],(long)out[i][3],
          out[i][4],out[i][5],out[i][6],out[i][7],out[i][8]),out[i][9];
      }
#endif
      fclose(debug_file);
    }
  }
}

long tcs_return_encoder_position(char axis)
 {
   char buffer[11];
   char number[21];
   int idx = 0;
   byte rbyte;

   #ifdef no_hardware
   return(0);
   #else
   if (axis=='z') return(0);

   sprintf(buffer, "a%c re;", axis);
   pc38_send_commands(buffer);
   do
     rbyte = pc38_read_byte();
   while ((rbyte == 0) || (rbyte == 10) || (rbyte == 13));
   number[idx++] = rbyte;

   do
     {
       rbyte = pc38_read_byte();
       if ((rbyte != 0) && (rbyte != 10) && (rbyte != 13))
         number[idx++] = rbyte;
     }
   while (rbyte != 10);
   number[idx] = 0;

   do
     rbyte = pc38_read_byte();
   while (rbyte);

   return atol(number);
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_return_step_position
//
//  Purpose....: return the PC-38 step counter
//
//               NOTE:  This function returns the actual PC-38 counter.
//                     The geartrain reversals ARE NOT figured into the
//                     return value.
//
// Input......:  axis - 'x', 'y', 'z', 't', 'u', 'v'
//
// Output.....:  axis counter
//
//------------------------------------------------------------------
long tcs_return_step_position(char axis)
 {
   char buffer[11];
   char number[21];
   int idx = 0;
   byte rbyte;

   #ifdef no_hardware
   return(0);
   #else
   sprintf(buffer, "a%c rp;", axis);
   pc38_send_commands(buffer);
   do
     rbyte = pc38_read_byte();
   while ((rbyte == 0) || (rbyte == 10) || (rbyte == 13));
   number[idx++] = rbyte;

   do
     {
       rbyte = pc38_read_byte();
       if ((rbyte != 0) && (rbyte != 10) && (rbyte != 13))
         number[idx++] = rbyte;
     }
   while (rbyte != 10);
   number[idx] = 0;

   do
     rbyte = pc38_read_byte();
   while (rbyte);

   return atol(number);
   #endif
 }

void writeline(char *string, int n)
{ 
  fprintf(stderr,"%s\n",string);
}

// PC-38
#define pc38_data_reg                   0x300
#define pc38_done_reg           0x301
#define pc38_control_reg        0x302
#define pc38_status_reg         0x303

// PC-34
#define pc34_data_reg                   0x304
#define pc34_done_reg                   0x305
#define pc34_control_reg        0x306
#define pc34_status_reg         0x307

// both
#define cmderr_bit      0x01
#define init_bit                0x02
#define enc_bit                 0x04
#define ovrt_bit                0x08
#define done_bit                0x10
#define ibf_bit                 0x20
#define tbe_bit                 0x40
#define irq_bit                 0x80


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
                sprintf(outbuf,"%s\n",commands);
                writeline(outbuf,4);
                if (!strlen(commands))
                        return;
#ifdef no_hardware
                return;
#else

                BOOL do_echo = (commands[0] == 1);      // \x01 means echo the command
                int idx = (int)do_echo;
        // if echo, skip first byte
                char *msg = strupr(strdup(commands));

                do
                        {
                                pcx_write_byte(card, msg[idx]);
                                if (do_echo)
                                        printf("%c", msg[idx]);
                        }
                while (msg[++idx]);
                if (do_echo)
                        printf("\n");
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
#ifdef no_hardware
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
                while (pcx_read_byte(card));
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
#ifdef no_hardware
                return(0);
#else
        if (card == PC38)
                return ((inportb(pc38_status_reg) & ibf_bit) ?
                        inportb(pc38_data_reg) : 0);
        else
                return ((inportb(pc34_status_reg) & ibf_bit) ?
                        inportb(pc34_data_reg) : 0);
#endif
        }




