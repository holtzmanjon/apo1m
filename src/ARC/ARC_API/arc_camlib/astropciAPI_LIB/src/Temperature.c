/******************************************************************************
*
*   File:       Temperature.c
*   Version:    1.00
*   Author:     Jose Maria Panero Ciprian
*   Abstract:   PCI DSP C library for functions to manage and convert between
*		temperature and adu units.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       08/17/00        jmp     1.00    Initial
*	01/03/02	sds	1.7	Modified to be a win2k static library.
*
******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "Temperature.h"  
#include "DSPCommand.h"  
#include "Bool.h"
#include "../WhichSystem.h"

#ifndef WIN2K
	#include <errno.h>
	/*#include <sys/mman.h>*/
	/*#include <fcntl.h>*/
#endif

/*****************************************
*  by default the algorithm is nonlinear
*****************************************/
char *algorithm = (char *)NONLINEAR;
double linear_coeff_0 = (double)0;
double linear_coeff_1 = (double)0;

/*******************************************************************************
*
*       Function:
*       ---------
*		char *get_temperature_algorithm () 
*
*       Description:
*       ------------
*       This function returns an string which is the current algorithm. The
*	current algorithm should be "linear" or "nonlinear". These values 
*	are supported by the constants LINEAR and NONLINEAR in the file
*	"Temperature.h".
*
*       Parameters:
*       -----------
*	None.
*
*       Returns:
*       --------
*       Returns a double which is the current linear coefficient 0 value.
*
*       Other effects:
*       --------------
*       None.
*
*
*       Version: 1.00
*       Author:  Jose Maria Panero
*       Date: 08/18/2000
*
*******************************************************************************/
char *get_temperature_algorithm ()
{
	return (char *)algorithm;
}

/*******************************************************************************
*
*       Function:
*       ---------
*		double get_temperature_linear_coeff_0 () 
*
*       Description:
*       ------------
*       This function returns a double which is the current 
*	linear coefficient 0 value.
*
*       Parameters:
*       -----------
*	None.
*
*       Returns:
*       --------
*       Returns a double which is the current linear coefficient 0 value.
*
*       Other effects:
*       --------------
*       None.
*
*
*       Version: 1.00
*       Author:  Jose Maria Panero
*       Date: 08/18/2000
*
*******************************************************************************/
double get_temperature_linear_coeff_0()
{
	return (double)linear_coeff_0;
}

/*******************************************************************************
*
*       Function:
*       ---------
*		double get_temperature_linear_coeff_1 () 
*
*       Description:
*       ------------
*       This function returns a double which is the current 
*	linear coefficient 1 value.
*
*       Parameters:
*       -----------
*	None.
*
*       Returns:
*       --------
*       Returns a double which is the current linear coefficient 1 value.
*
*       Other effects:
*       --------------
*       None.
*
*
*       Version: 1.00
*       Author:  Jose Maria Panero
*       Date: 08/18/2000
*
*******************************************************************************/
double get_temperature_linear_coeff_1()
{
	return (double)linear_coeff_1;
}

/*******************************************************************************
*
*       Function:
*       ---------
*		int set_algorithm (char *algorithm_to)
*
*       Description:
*       ------------
*	Determines whether to use the linear or non-linear algorithm. The 
*	algorithm is specified in the parameters, and may be equal to the
*	constants LINEAR or NONLINEAR. By default, the algorithm is NONLINEAR.
*
*	If not LINEAR is specified neither NONLINEAR, the function returns
*	_ERROR. Notice that the function doesn't take any action, simply 
*	returns the _ERROR flag.
*
*       Parameters:
*       -----------
*	algorithm_to	defines the algorithm to use to calculate the adu
*			representation. Algorithms are described by the
*			constants LINEAR and NONLINEAR in the file
*			"Temperature.h"
*
*       Returns:
*       --------
*       Returns the integer value _NO_ERROR
*
*       Other effects:
*       --------------
*       None.
*
*
*       Version: 1.00
*       Author:  Jose Maria Panero
*       Date: 08/17/2000
*
*******************************************************************************/
int set_algorithm (char *algorithm_to)
{

	int result = _NO_ERROR;

	/************
	*  if the specified algorithm is LINEAR set algorithm to LINEAR
	************/
	if ( strcmp(algorithm_to, LINEAR) == 0 )
        {
		algorithm = (char *)LINEAR;
        }
	else if ( strcmp(algorithm_to, NONLINEAR) == 0 )
	{
		/************
		*  by default the algorithm is nonlinear
		************/
		algorithm = (char *)NONLINEAR;
	}
	else
	{
		/************
		*  if the specified algorithm was not LINEAR neither NONLINEAR
		*  raise the flag result to _ERROR
		************/
		result = _ERROR;
	}

	/************
	*  return the result
	************/
	return result;
}

/*******************************************************************************
*
*       Function:
*       ---------
*		int set_linear_coefficients(double coeff0, double coeff1)
*
*       Description:
*       ------------
*	Sets the coefficients to use with the linear algorithm. For the linear
*	algorithm there's two coefficients, which are selected for the user,
*	and hence them should be set.
*
*       Parameters:
*       -----------
*	coeff0		a double for the "linear algorithm" coefficient 0
*	coeff1		a double for the "linear algorithm" coefficient 1
*
*       Returns:
*       --------
*       Returns the integer value _NO_ERROR.
*
*       Other effects:
*       --------------
*       None.
*
*
*       Version: 1.00
*       Author:  Jose Maria Panero
*       Date: 08/17/2000
*
*
*******************************************************************************/
int set_linear_coefficients(double coeff0, double coeff1)
{
	int result = _NO_ERROR;

	linear_coeff_0 = (double)coeff0;
	linear_coeff_1 = (double)coeff1;

	return result;
}

/******************************************************************************
*
*       Function:
*       ---------
*               int calculate_adu(double temperature)
*
*       Description:
*       ------------
*       Convert a double value representing a temperature in Celsius to the
*       corresponding ADU. Return ADU as an int value.
*
*       Parameters:
*       -----------
*       temperature     temperature in Celsius to be converted to ADU
*
*       algorithm       defines the algorithm to use to calculate the adu
*                       representation. Algorithms are described by the
*                       constants LINEAR and NONLINEAR in the file
*                       "Temperature.h"
*
*       Returns:
*       --------
*       Returns the conversion of the parameter temperature (Celsius) to ADU.
*
*       Other effects:
*       --------------
*       None.
*
*
*       Version: 1.00
*       Author:  Jose Maria Panero
*       Date: 08/03/2000
*
*       Revision History:
*       -----------------
*       Date            Who   Version    Description
*       ----------------------------------------------------------------------
*       08/03/2000      jmp     1.00    Initial
*
*	08/17/2000	jmp	1.00	Upgrade to support "linear" and 
*					"nonlinear" algorithms
*
*
*****************************************************************************/
int calculate_adu(double temperature)
{
    int tolerance;
    int trials = 0;
    double adu = 0.0;
    double vmid;
    double lower;
    double upper;
    double voltage = 0.0;
    double target_temp = KELVIN_TO_CELSIUS;

	/************
	*  case when the algorithm to apply is the linear algorithm.
	************/
        if ( strcmp(algorithm, LINEAR) == 0 )
        {
		adu= (temperature - linear_coeff_0) / linear_coeff_1;
        	return (int)adu;
        }

	/************
	*  by default the algorithm is nonlinear
	************/
        lower = UNIT_LOWER;
        upper = UNIT_UPPER;
        tolerance = FALSE;
        vmid = (lower + upper)*0.5;

        /**************
        * Numerically determine v for a given temperature
        * Choose an initial adu in the middle of vu and vl
        **************/
        adu = vmid * ADU_PER_VOLT + (double)ADU_OFFSET;

        /*if (fabs(target_temp - temperature) > TOLERANCE)
                tolerance = TRUE;*/

        while (tolerance && (trials < MAX_TOLERANCE_TRIALS))
        {
                target_temp = calculate_temperature((int)adu);

                /*if (fabs(target_temp - temperature) > TOLERANCE)
                        tolerance = TRUE;*/

                if (tolerance)
                {
                        if (target_temp < temperature)
                        {
                                upper = vmid;
                                vmid = (lower + upper)*0.5;
                                adu = vmid * ADU_PER_VOLT + (double)ADU_OFFSET;
                        }
                        else if (target_temp > temperature)
                        {
                                lower = vmid;
                                vmid = (lower + upper)*0.5;
                                adu = vmid * ADU_PER_VOLT + (double)ADU_OFFSET;
                        }
                }

                trials++;
        }

        return (int)adu;
}

/******************************************************************************
*
*       Function:
*       ---------
*               double calculate_temperature(int adu)
*
*       Description:
*       ------------
*       Convert a int value representing a temperature in ADU to the
*       corresponding temperature value in Celsius.
*       Return Celsius degrees as a double value.
*
*       Parameters:
*       -----------
*       adu             ADU value to be converted to temperature in Celsius
*
*       Returns:
*       --------
*       The conversion of the parameter in ADU to temperature (Celsius).
*
*       Other effects:
*       --------------
*       None.
*
*
*       Version: 1.00
*       Author:  Jose Maria Panero
*       Date: 08/03/2000
*
*       Revision History:
*       -----------------
*       Date            Who   Version    Description
*       ----------------------------------------------------------------------
*       08/03/2000      jmp     1.00    Initial
*
*	08/17/2000	jmp	1.00	Upgrade to support "linear" and 
*					"nonlinear" algorithms
*
*
*****************************************************************************/
double calculate_temperature(int adu)
{
        double temperature = KELVIN_TO_CELSIUS;
        double voltage = 0.0;
        double temperature_coeff[NUMBER_OF_COEFFS];
        double tc[NUMBER_OF_COEFFS];
        double x = 0.0;
        int i;                  /*  loop  */


	/************
	*  case when the algorithm to apply is the linear algorithm.
	************/
        if ( strcmp(algorithm, LINEAR) == 0 )
        {
		temperature= 
			linear_coeff_0 + linear_coeff_1 * ((double)adu);
        	return (double)temperature;
        }

        /********
        * Initialize the temperature coefficients array
        ********/
        temperature_coeff[0] = TEMP_COEFF_0;
        temperature_coeff[1] = TEMP_COEFF_1;
        temperature_coeff[2] = TEMP_COEFF_2;
        temperature_coeff[3] = TEMP_COEFF_3;
        temperature_coeff[4] = TEMP_COEFF_4;
        temperature_coeff[5] = TEMP_COEFF_5;
        temperature_coeff[6] = TEMP_COEFF_6;
        temperature_coeff[7] = TEMP_COEFF_7;
        temperature_coeff[8] = TEMP_COEFF_8;
        temperature_coeff[9] = TEMP_COEFF_9;
        temperature_coeff[10] = TEMP_COEFF_10;

        /********
        *  Convert adu's to voltage.
        ********/
        voltage = (adu - (double) ADU_OFFSET) / ADU_PER_VOLT;

        /********
        *  Calculate dimensionless variable for the Chebychev series.
        ********/
        x=((voltage-UNIT_LOWER)-(UNIT_UPPER-voltage))/(UNIT_UPPER-UNIT_LOWER);

        tc[0] = 1;
        tc[1] = x;

        if (NUMBER_OF_COEFFS <= 2 )
        {
                temperature += temperature_coeff[0]+(temperature_coeff[1] * x);
        }
        else
        {
                temperature += temperature_coeff[0]+(temperature_coeff[1] * x);
                for (i=2; i<NUMBER_OF_COEFFS; i++)
                {
                        tc[i] = 2.0 * x * tc[i-1] - tc[i-2];
                        temperature += temperature_coeff[i] * tc[i];
                }
        }

        return temperature;
}
