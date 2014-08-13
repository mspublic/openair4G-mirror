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
  
  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
#include "defs.h"
#include "linux/kernel.h"
#include "linux/module.h"

#ifdef BIGPHYSAREA
#ifdef ARCH_64 
char *bigphys_ptr,*bigphys_current;
#else //ARCH_64
unsigned int bigphys_ptr,bigphys_current;
#endif //ARCH_64

extern int exit_openair;

// return pointer to memory in big physical area aligned to 16 bytes

void *bigphys_malloc(n) {

  //printk("[BIGPHYSAREA] Calling bigphys_malloc\n");

  int n2 = n + ((16-(n%16))%16);

  bigphys_current += n2;

  if (bigphys_current > bigphys_ptr + (BIGPHYS_NUMPAGES*4096)) {
    printk("[BIGPHYS][FATAL] Memory overload!!!!! Exiting.\n");
    exit_openair = 1;
  }

#ifdef ARCH_64 
  //printk("[BIGPHYSAREA] Allocated Memory %d\n",bigphys_current-bigphys_ptr);
  return ((void *)(bigphys_current - (char *)n2));
#else //ARCH_64
  //printk("[BIGPHYSAREA] Allocated Memory %d\n",bigphys_current-bigphys_ptr);
  return ((void *)(bigphys_current - n2));
#endif //ARCH_64
}

EXPORT_SYMBOL(bigphys_malloc);
#endif



