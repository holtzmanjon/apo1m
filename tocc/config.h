/*************************************************************************/
/*							*/
/*	Module:		config.h			*/
/*							*/
/*	Purpose:	system configuration file base class	*/
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
#ifndef _CONFIG_H
	#define _CONFIG_H

#include <stdio.h>
#include "mytype.h"

//------------------------------------------------------------------
// 	Class......: TConfig
//
//  Purpose....: base class for system configuration files
//
//------------------------------------------------------------------
class TConfig
	{
		public:

		TConfig(const char *aFilename);
		~TConfig();

		virtual void initialize() = 0;	// !!! MUST OVERRIDE !!!
		virtual BOOL readData();
		virtual BOOL writeData();

		protected:
		virtual void* getDataPtr() { return NULL; }
		virtual int getDataSize() { return sizeof(*this); }

		private:
		char	*filename;
		FILE	*scfFile;
	};
#endif
/********************************* EOF **************************************/

