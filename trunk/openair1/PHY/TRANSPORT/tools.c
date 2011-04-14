/*________________________mac_utils.c________________________

 Authors : Raymond Knopp
 Company : EURECOM
 Emails  : knopp@eurecom.fr
________________________________________________________________*/

#ifndef USER_MODE
#include <linux/kernel.h>
#include <linux/module.h>
#endif //USER_MODE

#include "PHY/defs.h"
#include "PHY/types.h"
#include "PHY/extern.h"

#include "MAC_INTERFACE/extern.h"

//#define DEBUG_PHY

unsigned char conv_alloc_to_tb(unsigned char node_type,
			       unsigned char time_alloc,
			       unsigned short freq_alloc,
			       unsigned char coding_fmt,
			       unsigned short tb_size_bytes) {
  
  unsigned char number_of_symbols,number_of_groups,coding_rate,ntb_min,ntb_max;
  unsigned short sacch_size_input_bits,sach_size_bits,tb_size_bits;
  unsigned short i;
  unsigned short Res_min,Res_max, Eff_rate_min, Eff_rate_max;
  number_of_symbols = (time_alloc & 0xf)<<2;

  number_of_groups = 0;

  for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
    if ((freq_alloc & (1<<i)) > 0)  {// This freq group is allocated to the sach
      number_of_groups++;
    }
  }
  
  switch (coding_fmt&0x7) {
    
  case 0: // QPSK Single stream : Rate 1
    coding_rate = 2;
    break;
  case 1: // 16-QAM Single stream : Rate 2
  case 3: // QPSK Dual stream     : Rate 2
    coding_rate = 4;
    break;
  case 2: // 64-QAM Single stream : Rate 3
    coding_rate = 6;
    break;
  case 4: // 16-QAM Dual stream : Rate 4
    coding_rate = 8;
    break;
  case 5: // 64-QAM Dual stream : Rate 6
    coding_rate = 12;
    break;
  default:
    coding_rate = 2;
    break;
    
  }
  
  if (node_type == 0) // (i.e. without SACCH)
    sacch_size_input_bits = 0;
  else
    sacch_size_input_bits = SACCH_SIZE_BITS;
  
  // Sach size after rate matching, including SACCH (i.e. on channel)
  sach_size_bits = (((number_of_groups * NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP)*number_of_symbols)-sacch_size_input_bits)*(coding_rate);
  
  // Raw bits per TB
  tb_size_bits = (tb_size_bytes+4)<<3;
  
  ntb_min = (sach_size_bits/(tb_size_bits*2));  // Num TBs for rate 1/2
  ntb_max = (sach_size_bits/(10*tb_size_bits/7));  // Num TBs for rate 7/10
  
  
  Res_min=((sach_size_bits)-(ntb_min*tb_size_bits*2))/2;  // Num remaining uncoded bits that we can add to sach_size_bit at  rate 1/2 (less than LAYER2 1 TB)
  Res_max= ((sach_size_bits) -(ntb_max*10*tb_size_bits/7))*7/10;// Num remaining uncoded bits for rate 7/10
  
 
  Eff_rate_min=(100*(ntb_min*tb_size_bits*coding_rate))/(sach_size_bits);//Effective used rate , may be less than 0.5!!! 
  Eff_rate_max=(100*(ntb_max*tb_size_bits*coding_rate))/(sach_size_bits);
  
  
  if( Res_min > 0 ) 
    ntb_min++;
  if(ntb_min > ntb_max)
    ntb_min=0;
  


#ifdef DEBUG_PHY  
  msg("conv_alloc_to_tb (RBs %d,Symb %d,CF %d): sach_size_bits %d, sacch_size_bits %d, tb_size_bits %d, Nb_tb_min= %d, Nb_tb_max=%d, Res_min=%d,Res_max %d, Eff_rate_min %d,Eff_rate_max %d\n",
      number_of_groups,
      number_of_symbols,
      coding_rate,
      sach_size_bits,
      sacch_size_input_bits,
      tb_size_bits,
      ntb_min,
      ntb_max,
      Res_min>>3,
      Res_max>>3,
      Eff_rate_min,
      Eff_rate_max);
#endif //DEBUG_PHY



  return(ntb_min);

  if (coding_rate == 6)
    return(ntb_min);
  else
    return (ntb_max);
  
}


int conv_alloc_to_tb2(unsigned char node_type,
		      unsigned char time_alloc,
		      unsigned short freq_alloc,
		      unsigned char target_spec_eff,
		      unsigned char dual_stream_flag,
		      unsigned char num_tb_max,
		      unsigned char *coding_fmt,
		      unsigned char *num_tb,
		      unsigned short tb_size_bytes) {
  
  unsigned char number_of_symbols,number_of_groups,coding_rate,ntb_min,ntb_max;
  unsigned short sacch_size_input_bits,sach_size_bits,tb_size_bits;
  unsigned short i;
  unsigned short Res_min,Res_max, Eff_rate_min, Eff_rate_max;
  number_of_symbols = (time_alloc & 0xf)<<2;

  number_of_groups = 0;

  for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
    if ((freq_alloc & (1<<i)) > 0)  {// This freq group is allocated to the sach
      number_of_groups++;
    }
  }
  
#define SE_QPSK_SINGLE_MIN 0
#define SE_QPSK_SINGLE_MAX 4
#define SE_16QAM_SINGLE_MIN 5
#define SE_16QAM_SINGLE_MAX 25
#define SE_64QAM_SINGLE_MIN 26
#define SE_64QAM_SINGLE_MAX 46
#define SE_QPSK_DUAL_MIN 5
#define SE_QPSK_DUAL_MAX 25
#define SE_16QAM_DUAL_MIN 26
#define SE_16QAM_DUAL_MAX 48
#define SE_64QAM_DUAL_MIN 49
#define SE_64QAM_DUAL_MAX 107
  // compute coding_fmt from target spectral efficiency
  // spectral efficiency is coded as SE2 = 16*(SE - 1), this gives increments of .0625 starting from 1 bps/Hz
  // i.e. SE2 = 0 for  SE=1 bps/Hz       (QPSK Single)
  //      SE2 = 1 for  SE=1.0625 bps/Hz  (QPSK Single)
  //      SE2 = 2 for  SE=1.125 bps/Hz   (QPSK Single)
  //      SE2 = 3 for  SE=1.1875 bps/Hz  (QPSK Single)
  //      SE2 = 4 for  SE=1.25 bps/Hz    (QPSK Single)
  //      SE2 = 5 for  SE=1.3125 bps/Hz  (16QAM Single,QPSK Dual)
  //      SE2 = 6 for  SE=1.375 bps/Hz   (16QAM Single,QPSK Dual)
  //      SE2 = 7 for  SE=1.4375 bps/Hz  (16QAM Single,QPSK Dual)
  //      SE2 = 8 for  SE=1.5 bps/Hz     (16QAM Single,QPSK Dual)
  //      SE2 = 16 for SE=2 bps/Hz       (16QAM Single,QPSK Dual)
  //      ..
  //      SE2 = 25 for SE=2.5625 bps/Hz  (16QAM Single,QPSK Dual)
  //      SE2 = 26 for SE=2.6250 bps/Hz  (64QAM Single,16QAM Dual)
  //      ...
  //      SE2 = 32 for SE=3 bps/Hz       (64QAM Single,16QAM Dual)
  //      SE2 = 46 for SE=3.876 bps/Hz   (64QAM Single,16QAM Dual)
  //      SE2 = 47 for SE=3.9375 bps/Hz  (16QAM Dual)
  //      SE2 = 48 for SE=4 bps/Hz       (16QAM Dual)
  //      SE2 = 49 for SE=4.0625 bps/Hz  (64 QAM Dual, HA HA HA!!)
  //      ...
  //      SE2 = 107 for SE=6.6875 bps/Hz (64 QAM Dual, HA HA HA!!)

  if (dual_stream_flag == 1) {

    if (target_spec_eff < SE_QPSK_DUAL_MIN) {
      msg("[PHY][TRANSPORT][TOOLS] Illegal Spectral Efficiency (%d) for dual-stream, exiting\n");
      return(-1);
    }
    else if (target_spec_eff < SE_16QAM_DUAL_MIN)
      *coding_fmt = 3;
    else if (target_spec_eff < SE_64QAM_DUAL_MIN)
      *coding_fmt = 4;
    else if (target_spec_eff <= SE_64QAM_DUAL_MAX)
      *coding_fmt = 5;
    else {
      msg("[PHY][TRANSPORT][TOOLS] Illegal Spectral Efficiency (%d > 107) exiting\n");
      return(-1);
    }
  }
  else { // single-stream
    if (target_spec_eff < SE_16QAM_SINGLE_MIN) {
      *coding_fmt = 0;
    }
    else if (target_spec_eff < SE_64QAM_SINGLE_MIN)
      *coding_fmt = 1;
    else if (target_spec_eff <= SE_64QAM_SINGLE_MAX)
      *coding_fmt = 2;
    else {
      msg("[PHY][TRANSPORT][TOOLS] Illegal Spectral Efficiency (%d > 46) for single-stream, exiting\n");
      return(-1);
    }
  } 



  switch (*coding_fmt&0x7) {
    
  case 0: // QPSK Single stream : Rate 1
    coding_rate = 2;
    break;
  case 1: // 16-QAM Single stream : Rate 2
  case 3: // QPSK Dual stream     : Rate 2
    coding_rate = 4;
    break;
  case 2: // 64-QAM Single stream : Rate 3
    coding_rate = 6;
    break;
  case 4: // 16-QAM Dual stream : Rate 4
    coding_rate = 8;
    break;
  case 5: // 64-QAM Dual stream : Rate 6
    coding_rate = 12;
    break;
  default:
    coding_rate = 2;
    break;
    
  }
  
  if (node_type == 0) // (i.e. without SACCH)
    sacch_size_input_bits = 0;
  else
    sacch_size_input_bits = SACCH_SIZE_BITS;
  
  // Sach size after rate matching, including SACCH (i.e. on channel)
  sach_size_bits = (((number_of_groups * NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP)*number_of_symbols)-sacch_size_input_bits)*(coding_rate);
  
  // Raw bits per TB
  tb_size_bits = (tb_size_bytes+4)<<3;


  *num_tb = (16+target_spec_eff)*sach_size_bits/((tb_size_bits*coding_rate)<<4);

#ifdef DEBUG_PHY  
  msg("conv_alloc_to_tb (RBs %d,Symb %d,SE %d, CF %d): sach_size_bits %d, sacch_size_bits %d, tb_size_bits %d, Nb_tb= %d, coding_fmt=%d\n",
      number_of_groups,
      number_of_symbols,
      target_spec_eff,
      coding_rate,
      sach_size_bits,
      sacch_size_input_bits,
      tb_size_bits,
      *num_tb,
      //      num_tb_max,
      *coding_fmt);
#endif //DEBUG_PHY



  return(0);

}

unsigned char conv_alloc_to_coding_fmt(unsigned char node_type,
				       unsigned char time_alloc,
				       unsigned short freq_alloc,
				       unsigned char target_spec_eff,
				       unsigned char dual_stream_flag,
				       //				       unsigned char num_tb_max,
				       unsigned char *coding_fmt,
				       unsigned char *num_tb,
				       unsigned short tb_size_bytes) {
  
  unsigned char number_of_symbols,number_of_groups,coding_rate,ntb_min,ntb_max;
  unsigned short sacch_size_input_bits,sach_size_bits,tb_size_bits;
  unsigned short i;
  unsigned short Res_min,Res_max, Eff_rate_min, Eff_rate_max;
  number_of_symbols = (time_alloc & 0xf)<<2;

  number_of_groups = 0;

  for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
    if ((freq_alloc & (1<<i)) > 0)  {// This freq group is allocated to the sach
      number_of_groups++;
    }
  }
  
#define SE_QPSK_SINGLE_MIN 0
#define SE_QPSK_SINGLE_MAX 4
#define SE_16QAM_SINGLE_MIN 5
#define SE_16QAM_SINGLE_MAX 25
#define SE_64QAM_SINGLE_MIN 26
#define SE_64QAM_SINGLE_MAX 46
#define SE_QPSK_DUAL_MIN 5
#define SE_QPSK_DUAL_MAX 25
#define SE_16QAM_DUAL_MIN 26
#define SE_16QAM_DUAL_MAX 48
#define SE_64QAM_DUAL_MIN 49
#define SE_64QAM_DUAL_MAX 107
  // compute coding_fmt from target spectral efficiency
  // spectral efficiency is coded as SE2 = 16*(SE - 1), this gives increments of .0625 starting from 1 bps/Hz
  // i.e. SE2 = 0 for  SE=1 bps/Hz       (QPSK Single)
  //      SE2 = 1 for  SE=1.0625 bps/Hz  (QPSK Single)
  //      SE2 = 2 for  SE=1.125 bps/Hz   (QPSK Single)
  //      SE2 = 3 for  SE=1.1875 bps/Hz  (QPSK Single)
  //      SE2 = 4 for  SE=1.25 bps/Hz    (QPSK Single)
  //      SE2 = 5 for  SE=1.3125 bps/Hz  (16QAM Single,QPSK Dual)
  //      SE2 = 6 for  SE=1.375 bps/Hz   (16QAM Single,QPSK Dual)
  //      SE2 = 7 for  SE=1.4375 bps/Hz  (16QAM Single,QPSK Dual)
  //      SE2 = 8 for  SE=1.5 bps/Hz     (16QAM Single,QPSK Dual)
  //      SE2 = 16 for SE=2 bps/Hz       (16QAM Single,QPSK Dual)
  //      ..  //      SE2 = 25 for SE=2.5625 bps/Hz  (16QAM Single,QPSK Dual)
  //      SE2 = 26 for SE=2.6250 bps/Hz  (64QAM Single,16QAM Dual)
  //      ...
  //      SE2 = 32 for SE=3 bps/Hz       (64QAM Single,16QAM Dual)
  //      SE2 = 46 for SE=3.876 bps/Hz   (64QAM Single,16QAM Dual)
  //      SE2 = 47 for SE=3.9375 bps/Hz  (16QAM Dual)
  //      SE2 = 48 for SE=4 bps/Hz       (16QAM Dual)
  //      SE2 = 49 for SE=4.0625 bps/Hz  (64 QAM Dual, HA HA HA!!)
  //      ...
  //      SE2 = 107 for SE=6.6875 bps/Hz (64 QAM Dual, HA HA HA!!)

  if (dual_stream_flag == 1) {

    if (target_spec_eff < SE_QPSK_DUAL_MIN) {
      msg("[PHY][TRANSPORT][TOOLS] Illegal Spectral Efficiency (%d) for dual-stream, exiting\n");
      return(-1);
    }
    else if (target_spec_eff < SE_16QAM_DUAL_MIN)
      *coding_fmt = 3;
    else if (target_spec_eff < SE_64QAM_DUAL_MIN)
      *coding_fmt = 4;
    else if (target_spec_eff <= SE_64QAM_DUAL_MAX)
      *coding_fmt = 5;
    else {
      msg("[PHY][TRANSPORT][TOOLS] Illegal Spectral Efficiency (%d > 107) exiting\n");
      return(-1);
    }
  }
  else { // single-stream
    if (target_spec_eff < SE_16QAM_SINGLE_MIN) {
      *coding_fmt = 0;
    }
    else if (target_spec_eff < SE_64QAM_SINGLE_MIN)
      *coding_fmt = 1;
    else if (target_spec_eff <= SE_64QAM_SINGLE_MAX)
      *coding_fmt = 2;
    else {
      msg("[PHY][TRANSPORT][TOOLS] Illegal Spectral Efficiency (%d > 46) for single-stream, exiting\n");
      return(-1);
    }
  } 



  switch (*coding_fmt&0x7) {
    
  case 0: // QPSK Single stream : Rate 1
    coding_rate = 2;
    break;
  case 1: // 16-QAM Single stream : Rate 2
  case 3: // QPSK Dual stream     : Rate 2
    coding_rate = 4;
    break;
  case 2: // 64-QAM Single stream : Rate 3
    coding_rate = 6;
    break;
  case 4: // 16-QAM Dual stream : Rate 4
    coding_rate = 8;
    break;
  case 5: // 64-QAM Dual stream : Rate 6
    coding_rate = 12;
    break;
  default:
    coding_rate = 2;
    break;
    
  }
  
  if (node_type == 0) // (i.e. without SACCH)
    sacch_size_input_bits = 0;
  else
    sacch_size_input_bits = SACCH_SIZE_BITS;
  
  // Sach size after rate matching, including SACCH (i.e. on channel)
  sach_size_bits = (((number_of_groups * NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP)*number_of_symbols)-sacch_size_input_bits)*(coding_rate);
  
  // Raw bits per TB
  tb_size_bits = (tb_size_bytes+4)<<3;


  *num_tb = (16+target_spec_eff)*sach_size_bits/((tb_size_bits*coding_rate)<<4);

  //#ifdef DEBUG_PHY  

  if (*coding_fmt > 1) {
    msg("conv_alloc_to_coding_fmt (RBs %d,Symb %d,SE %d, CF %d): sach_size_bits %d, sacch_size_bits %d, tb_size_bits %d, Nb_tb= %d, coding_fmt=%d\n",
	number_of_groups,
	number_of_symbols,
	target_spec_eff,
      coding_rate,
	sach_size_bits,
	sacch_size_input_bits,
	tb_size_bits,
	*num_tb,
	//  num_tb_max,
	*coding_fmt);
#ifndef USER_MODE
    mac_xface->macphy_exit("");
#endif
  }

  //#endif //DEBUG_PHY



  return(0);

}

#ifndef USER_MODE
EXPORT_SYMBOL(conv_alloc_to_tb);
EXPORT_SYMBOL(conv_alloc_to_tb2);
EXPORT_SYMBOL(conv_alloc_to_coding_fmt);
#endif //USER_MODE
