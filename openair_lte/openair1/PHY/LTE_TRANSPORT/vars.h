#include "dlsch_tbs.h"

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

unsigned short scfdma_amps[26] = {0,5120,3620,2956,2560,2290,2090,1935,1810,1706,1619,1544,1478,1420,1368,1322,1280,1242,1207,1175,1145,1117,1092,1068,1045,1024};
