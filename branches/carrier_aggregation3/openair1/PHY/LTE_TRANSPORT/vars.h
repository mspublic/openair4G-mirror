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
#include "dlsch_tbs.h"
//#include "dlsch_tbs_full.h"
#include "sss.h"

unsigned short lte_cqi_eff1024[16] = {156,          //-6, .15234
				      240,          //-4  .234
				      386,          //-2  .3769
				      616,          //1   .6016
				      898,          //4   .87695
				      1204,         //6   1.1758
				      1512,         //8   1.4766
				      1960,         //11  1.9141
				      2464,         //14  2.4062
				      2796,         //16  2.7305
				      3402,         //20  3.3223
				      3996,         //23  3.9023
				      4632,         //27  4.5234
				      5238,         //31  5.1152
				      5688};        //33  5.5547
 
char lte_cqi_snr_dB[15] = { -2,
			    0,
			    1,
			    2,
			    4,
			    6,
			    10,
			    14,
			    18,
			    22,
			    26,
			    30,
			    34,
			    38,
			    40};

unsigned char ue_power_offsets[25] = {14,11,9,8,7,6,6,5,4,4,4,3,3,3,2,2,2,1,1,1,1,1,0,0,0};

short conjugate[8]__attribute__((aligned(16))) = {-1,1,-1,1,-1,1,-1,1} ;
short conjugate2[8]__attribute__((aligned(16))) = {1,-1,1,-1,1,-1,1,-1} ;

int qam64_table[8],qam16_table[4];

unsigned char cs_ri_normal[4]    = {1,4,7,10};
unsigned char cs_ri_extended[4]  = {0,3,5,8};
unsigned char cs_ack_normal[4]   = {2,3,8,9};
unsigned char cs_ack_extended[4] = {1,2,6,7};

//unsigned short scfdma_amps[25] = {0,5120,3620,2956,2560,2290,2090,1935,1810,1706,1619,1544,1478,1420,1368,1322,1280,1242,1207,1175,1145,1117,1092,1068,1045,1024};

char dci_format_strings[15][13] = {"0","1","1A","1B","1C","1D","1E_2A_M10PRB",
				   "2","2A","2B","2C","2D",
				   "3"};

uint8_t wACK[5][4] = {{1,1,1,1},{1,0,1,0},{1,1,0,0},{1,0,0,1},{0,0,0,0}};
int8_t wACK_RX[5][4] = {{-1,-1,-1,-1},{-1,1,-1,1},{-1,-1,1,1},{-1,1,1,-1},{1,1,1,1}};
