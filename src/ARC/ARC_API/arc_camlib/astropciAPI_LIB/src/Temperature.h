/*******************************************************************************
*
*   File:       Temperature.h
*   Version:    1.00
*   Author:     Jose Maria Panero Ciprian
*   Abstract:	C library for the array temperature coefficients for the 
*		SDSU ADC board.
*   References: The references for this include file are, the previous include
*		file by Scott Streit, and the SDSU ADC board documentation.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       07/10/00        jmp     1.00    Initial
*
*******************************************************************************/

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

/*******************************************************************************
*
*	Temperature Coefficient Constants
*	=======================================
*
*	Cicada Conversion of ADU to Temperature
*	=======================================
*
*	Set PhysicalSensorUnit to voltage. 
*	The SDSU ADC is 12 bit and has following conversion of ADU to voltage:
*
*	----------------------------------------------------------------------
*	hex		ADC voltage 	unsigned decimal	Diode Voltage 
*	----------------------------------------------------------------------
*	FFF		+3V			4096			+1.5
*	7FF	 	 0V			2048			 0
*	000		-3V		   	   0			-1.5
*	----------------------------------------------------------------------
*
*	Therefore, the following SDSU parameters will convert ADUs to voltage 
*
*	ADUoffset = 2048 
*	ADUperUnit = 2048/1.5 = 1365.33 ADU/VOLT 
*
*	To convert voltage to temperature, the SDSU software uses omega 
*	representation of curve #10 by a polynomial equation based
*	on chebychev polynomials.
*
*	For temperature range 100...475K, the following parameters are needed:
*
*		UnitLower = 0.079767 volts 
*		UnitUpper = 0.999614 volts 
*
*		NumberTemperatureCoeffs = 11 
*	
*		TemperatureCoefficient(0) = 287.757 
*		TemperatuteCoefficient(1) = -194.145 
*		TemperatuteCoefficient(2) = -3.8379 
*		TemperatuteCoefficient(3) = -1.31833 
*		TemperatuteCoefficient(4) = -0.10912 
*		TemperatuteCoefficient(5) = -0.393265 
*		TemperatuteCoefficient(6) = 0.146911 
*		TemperatuteCoefficient(7) = -0.111192 
*		TemperatuteCoefficient(8) = 0.028877 
*		TemperatuteCoefficient(9) = -0.029286 
*		TemperatuteCoefficient(10) = -0.029286 
*		TemperatuteCoefficient(11) = 0.015619 
*
*		IncreaseWithTemperature = false
*
*	(Voltage produced by diodes decreases with temperature)
*
******************************************************************************/
#define ADU_PER_VOLT		1365.33		/* 1366.98 */
#define ADU_OFFSET		2048		/* 2045 */

#define UNIT_LOWER		0.079767
#define UNIT_UPPER		0.999614

#define NUMBER_OF_COEFFS	11
#define TEMP_COEFF_0		287.756797
#define TEMP_COEFF_1		-194.144823
#define TEMP_COEFF_2		-3.837903
#define TEMP_COEFF_3		-1.318325
#define TEMP_COEFF_4		-0.109120
#define TEMP_COEFF_5		-0.393265
#define TEMP_COEFF_6		0.146911
#define TEMP_COEFF_7		-0.111192
#define TEMP_COEFF_8		0.028877
#define TEMP_COEFF_9		-0.029286
#define TEMP_COEFF_10		0.015619

#define INCREASE_WITH_TEMP	0

#define KELVIN_TO_CELSIUS	(double)-273.15

#define TOLERANCE		(double)-0.5
#define MAX_TOLERANCE_TRIALS	(double)30

/**************************************************************************
*  The two possible algorithms to convert between temperature Celsius and
*  adu, are "linear" (user input coefficients) and "nonlinear".
***************************************************************************/
#define	LINEAR			(char *)"linear"
#define	NONLINEAR		(char *)"nonlinear"

/*************************
*  C functions prototypes
**************************/
char *get_temperature_algorithm();
double get_temperature_linear_coeff_0();
double get_temperature_linear_coeff_1();
int set_algorithm(char *algorithm_to);
int set_linear_coefficients(double coeff0, double coeff1);
int calculate_adu(double temperature);
double calculate_temperature(int adu);

#endif
