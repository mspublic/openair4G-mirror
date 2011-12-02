/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/
/*! \file taus.c
* \brief random number generator per OAI component 
* \author Raymond Knopp and Navid Nikaein
* \date 2011
* \version 0.1
* \warning 
* @ingroup util
*/

#ifndef RTAI_ENABLED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#else
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai.h>
#include <rtai_sched.h>
#define time(x) (unsigned int)(rt_get_time_ns())
#endif

typedef enum {MIN_NUM_COMPS=0, PHY, OMG, OCM, OTG, MAX_NUM_COMPS} comp_t;

unsigned int s0[MAX_NUM_COMPS], s1[MAX_NUM_COMPS], s2[MAX_NUM_COMPS], b[MAX_NUM_COMPS], r[MAX_NUM_COMPS];




inline unsigned int taus(unsigned int comp) {

  b[comp] = (((s0[comp] << 13) ^ s0[comp]) >> 19);
  s0[comp] = (((s0[comp] & 0xFFFFFFFE) << 12)^  b[comp]);
  b[comp] = (((s1[comp] << 2) ^ s1[comp]) >> 25);
  s1[comp] = (((s1[comp] & 0xFFFFFFF8) << 4)^  b[comp]);
  b[comp] = (((s2[comp] << 3) ^ s2[comp]) >> 11);
  s2[comp] = (((s2[comp] & 0xFFFFFFF0) << 17)^  b[comp]);
  r[comp] = s0[comp] ^ s1[comp] ^ s2[comp]; 
  return r[comp];
}

void set_taus_seed(unsigned int seed_type) {
  
  unsigned int i; // i index of component 
  
  for (i=MIN_NUM_COMPS; i < MAX_NUM_COMPS  ; i ++)	{
    
    switch (seed_type){
    case 0: // use rand func 
      if (i == 0) srand(time(NULL));
      s0[i] = ((unsigned int)rand());
      s1[i] = ((unsigned int)rand());
      s2[i] = ((unsigned int)rand());
      printf("Initial seeds use rand: s0[%d] = 0x%x, s1[%d] = 0x%x, s2[%d] = 0x%x\n", i, s0[i], i, s1[i], i, s2[i]);
      break;
    case 1: // use rand with seed
      if (i == 0) srand(0x1e23d851);
      s0[i] = ((unsigned int)rand());
      s1[i] = ((unsigned int)rand());
      s2[i] = ((unsigned int)rand());
      printf("Initial seeds use rand with seed : s0[%d] = 0x%x, s1[%d] = 0x%x, s2[%d] = 0x%x\n", i, s0[i], i, s1[i], i, s2[i]);
      
      break;
    default: 
      break;
      
    }
  }
}

int get_rand (unsigned int comp){
  if ((comp > MIN_NUM_COMPS) && (comp < MAX_NUM_COMPS))
    return r[comp];
  else{
    //LOG_E(RNG,"unknown component %d\n",comp);
    return -1;
  }
}

unsigned int dtaus(unsigned int comp, unsigned int a, unsigned b){
  
  return (int) (((double)taus(comp)/(double)0xffffffff)* (double)(b-a) + (double)a);
}


main() {

  unsigned int i,randomg, randphy;

  set_taus_seed(0);
  printf("dtaus %d \n",dtaus(PHY, 1000, 1000000));

  do {//for (i=0;i<10;i++){
    randphy = taus(PHY);	
    randomg = taus(OTG);
    i++;
    // printf("rand for OMG (%d,0x%x) PHY (%d,0x%x)\n",OMG, randomg, PHY, randphy);
  } while (randphy != randomg);	
  printf("after %d run: get rand for (OMG 0x%x, PHY 0x%x)\n",i, get_rand(OTG), get_rand(PHY));
  
}

