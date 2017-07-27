/****************************************************************************
 *
 * Project:     Pico ADC-10
 *
 * Filename:    adc10.h
 *
 * Copyright:   Pico Technology Limited 1994
 *
 * Author:      Mike Green
 *              Pico Technology Limited
 *              Broadway House
 *              149-151 St Neots Road
 *              Hardwick
 *              Cambridge CB3 7QJ UK
 *              Tel. +44 (0) 1954 211716
 *              Fax. +44 (0) 1954 211880
 *              Email: 100073,2365@compuserve.com
 *
 * Description:
 *
 * This header defines the interface to the advanced driver routines
 * for the PICO ADC-10.
 *
 * History:
 *  13Oct94 MKG New driver
 *
 * Revision Info: "file %n date %f revision %v"
 *                "file ADC10.H date 11-Feb-95,16:32:40 revision 1"
 *
 ****************************************************************************/

/* C++ and windows need different prefixes for the declarations...
 */

#ifdef __cplusplus
#define PREF1 extern "C"
#ifdef  WINDOWS
#define PREF2 _EXPORT
#else
#define PREF2
#endif
#else
#define PREF1
#define PREF2 far
#endif


        /* adc10_driver_open
         *
         * Call this routine once only to open the driver.
         *
         * This is the normal way of opening the driver...
         * you do not need to know the address of the port
         *
         */
PREF1 int PREF2 pascal adc10_driver_open
        (
        int port                /* 1 for printer port 1,        */
                                /* 2 for port 2 etc             */
        );


        /* adc10_driver_open_address
         *
         * Call this routine once only to open the driver.
         *
         * This routine can be used when the BIOS does not
         *  detect the port at power on if the ADC-10
         * is plugged in
         */
PREF1 int PREF2 pascal adc10_driver_open_address
        (
        unsigned int address    /* Address of printer port      */
                                /* normally 0x378 or 0x278      */
        );



        /* adc10_get_value
         *
         * This routine gets one reading from the adc
         *
         * It clocks TWO readings from the converter,
         *  because asking for the first reading
         *   starts conversion for the second reading
         *
         */
PREF1 int  PREF2 pascal adc10_get_value (void);



        /* adc10_get_values
         *
         * This routine gets a block of values from the ADC,
         * at time intervals specified by the most recent call
         * to adc10_set_interval.
         *
         * You can ask for a different number of samples for
         * each call: it does not have to be the same as the
         * ideal number of samples in the call to adc10_set_interval.
         *
         * The interval between samples will remain the same.
         */
PREF1 unsigned int PREF2 pascal adc10_get_values
        (
        int far * buffer,       /* Pointer to buffer for values */
        int no_of_values        /* number of values to collect  */
        );




        /* adc10_set_interval
         *
         * Specify the time interval between samples for the next
         * call to adc10_get_values.
         *
         * This routine returns the time that number of conversions
         * will actually take: the driver may not be able to go
         * as fast as you requested.
         *
         */

PREF1 unsigned long PREF2 pascal adc10_set_interval
        (
        unsigned long us_for_block,/* total time in micro-seconds        */
        int ideal_no_of_samples    /* No of samples to take in that time */
        );


#ifndef CM_DEFINED
#define CM_DEFINED
typedef enum {CM_AVERAGE, CM_MINIMUM, CM_MAXIMUM, CM_SUM, CM_MAX} COMBINATION_METHOD;
#endif



        /* adc10_get_combined_values
         *
         * This routine takes several readings from the
	 * ADC and combines the readings to give one long value
         *
         * The return value is 0..255
         *      0    = 0 Volts
         *      255 = 5 Volts
	 *
	 * For CM-SUM, it is no_of_readings times this
         */
PREF1 unsigned long PREF2 pascal adc10_get_combined_values
        (
        COMBINATION_METHOD  mode,   /* Combination modes (CM_XXX) */
        unsigned int readings_for_channels/* No of readings */
        );
