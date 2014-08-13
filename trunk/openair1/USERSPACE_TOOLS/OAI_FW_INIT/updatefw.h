/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/
/*
    C Header file <updatefw.h> for updatefw tool.

    K. Khalfallah, Aug, 2007
    kkhalfallah@free.fr
*/

#ifndef UPDATEFW_H
#define UPDATEFW_H

#include <stdio.h>
#include <errno.h>
#define _testsyscall(x, y, z)    {if (x==-1) {    \
                                    fprintf(stderr, "Error : %s() returned "z" (errno=%d)\n", y, x, errno);    \
                                    fprintf(stderr, "Warning : not leaving normally. Remove ipc ressources by hand !");    \
                                    fprintf(stderr, " (use shell commands ipcs & ipcrm)\n");    \
                                    exit(-1);    \
                                }}
//                                  else {    \
//                                    printf("Info : successfully called %s, returned "z"\n", y, x);}}

/* Intel is little-endian, sparc is big */
unsigned long ltmp;
#define invert4(x)      {ltmp=x; x=((ltmp & 0xff)<<24) | ((ltmp & 0xff00)<<8) | \
                         ((ltmp & 0xff0000)>>8) | ((ltmp & 0xff000000)>>24); }
unsigned short stmp;
#define invert2(x)		{stmp=x; x=((stmp & 0xff)<< 8) | ((stmp & 0xff00)>>8);}


/* static variables, global */
char* virt_addr;
unsigned int packet_ndx = 0;

#define FSEEK_ORIGIN        0
#define FSEEK_CURRENT        1
#define READ_FILE_MODE            "r"
/* Verbose levels */
#define VERBOSE_LEVEL_MAIN_STEPS        1
#define VERBOSE_LEVEL_ACCESS_FILES      2
#define VERBOSE_LEVEL_SECTION_DETAILS   3
#define VERBOSE_LEVEL_IOCTL             4


#endif /* UPDATEFW_H */
