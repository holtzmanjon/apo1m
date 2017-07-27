/*
This file is part of OpenLogos/LogOSMaTrans.  Copyright (C) 2005 Globalware AG

OpenLogos/LogOSMaTrans has two licensing options:

The Commercial License, which allows you to provide commercial software
licenses to your customers or distribute Logos MT based applications or to use
LogOSMaTran for commercial purposes. This is for organizations who do not want
to comply with the GNU General Public License (GPL) in releasing the source
code for their applications as open source / free software.

The Open Source License allows you to offer your software under an open source
/ free software license to all who wish to use, modify, and distribute it
freely. The Open Source License allows you to use the software at no charge
under the condition that if you use OpenLogos/LogOSMaTran in an application you
redistribute, the complete source code for your application must be available
and freely redistributable under reasonable conditions. GlobalWare AG bases its
interpretation of the GPL on the Free Software Foundation's Frequently Asked
Questions.

OpenLogos is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the License conditions along with this
program. If not, write to Globalware AG, Hospitalstraﬂe 6, D-99817 Eisenach.

Linux port modifications and additions by Bernd Kiefer, Walter Kasper,
Deutsches Forschungszentrum fuer kuenstliche Intelligenz (DFKI)
Stuhlsatzenhausweg 3, D-66123 Saarbruecken
*/
/* -*- Mode: C++ -*- */

#ifndef _GETPRIVATEPROFILESTRING_H
#define _GETPRIVATEPROFILESTRING_H
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 512

/**************************************************************************
* Function:     GetPrivateProfileString()
* Arguments:    <char *> section - the name of the section to search for
*               <char *> entry - the name of the entry to find the value of
*               <char *> def - default string in the event of a failed read
*               <char *> buffer - a pointer to the buffer to copy into
*               <int> buffer_len - the max number of characters to copy
*               <char *> file_name - the name of the .ini file to read from
* Returns:      the number of characters copied into the supplied buffer
***************************************************************************/
/******************************************************************************
 PORTABLE ROUTINES FOR WRITING PRIVATE PROFILE STRINGS --  by Joseph J. Graf
 Header file containing prototypes and compile-time configuration.
******************************************************************************/
#if defined(__STDC__) || defined(__cplusplus)
int 
GetPrivateProfileString(const char *section, const char *entry
                        , const char *def
                        , char *buffer, int buffer_len
                        , const char *file_name)
#else
int GetPrivateProfileString(section, entry, def, buffer, buffer_len, file_name)
char		*section,*entry,*def,*buffer,*file_name;
int		buffer_len;
#endif
{   
  char buff[MAX_LINE_LENGTH];
  char *ep;
  char t_section[MAX_LINE_LENGTH];
  char *ptr;
  int tlen;
  int len = strlen(entry);

  FILE *fp = fopen(file_name,"r");
  if( !fp ) return(0);

  sprintf(t_section,"[%s]",section);    /* Format the section name */
  tlen=strlen(t_section);

  /*  Move through file 1 line at a time until a section is matched or EOF */
  do
    {   if( fgets(buff,MAX_LINE_LENGTH-1,fp) == NULL)
      {   fclose(fp);
      strncpy(buffer,def,buffer_len);     
      return(strlen(buffer));
      }
    }
  while( strncasecmp(buff,t_section,tlen) );

  /* Now that the section has been found, find the entry.
   * Stop searching upon leaving the section's area. */
  do
    {   
      if ( fgets(buff,MAX_LINE_LENGTH-1,fp) == NULL)
        {   fclose(fp);
        strncpy(buffer,def,buffer_len);     
        return(strlen(buffer));
        }
    }  while( strncasecmp(buff,entry,len) );

  fclose(fp);

  ep = strrchr(buff,'=');    /* Parse out the equal sign */
  if (ep == NULL)
    {
      strncpy(buffer,def,buffer_len);     
      return(strlen(buffer));
    }
  ep++;

  /* remove leading spaces*/
  while (*ep && (isspace(*ep) || *ep == 10))
    ep++;
  if (ep == NULL)
    {
      strncpy(buffer,def,buffer_len);     
      return(strlen(buffer));
    }
	
  /* remove trailing spaces*/
  ptr = ep;
  while(*ptr) // go to the end, point to a NULL
    ptr++;

  ptr--;
  while (ptr > ep)  // backup and put in nulls if there is a space
    {
      if (isspace(*ptr) || *ep == 10)
        {
          *ptr = 0;
          ptr--;
        }
      else
        break;
    }

  /* Copy up to buffer_len chars to buffer */
  strncpy(buffer,ep,buffer_len - 1);

  buffer[buffer_len] = '\0';
  return(strlen(buffer));
}

#undef MAX_LINE_LENGTH

#endif
