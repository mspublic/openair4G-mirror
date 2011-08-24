/*!\brief SACH Encoding and Decoding*/
///
/// Calls underlying channel coding and modulation for SACH/SACCH/RACH
///

#ifndef USER_MODE
#define __NO_VERSION__



//#ifdef RTAI_ENABLED
//#include <rtai.h>
//#include <rtai_posix.h>
//#include <rtai_fifos.h>
//#endif

#else
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#endif

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif //CBMIMO1


#include "PHY/defs.h"
#include "PHY/types.h"
#include "PHY/extern.h"
#include "PHY/TOOLS/defs.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"

#ifdef OPENAIR2
#include "PHY_INTERFACE/defs.h"
#endif

#ifndef EXPRESSMIMO_TARGET
#include "xmmintrin.h"
#endif //EXPRESSMIMO_TARGET

#ifndef USER_MODE
#include <linux/crc32.h>
#else

#endif //



unsigned char first_sach_tx;

#ifdef OPENAIR2


void phy_generate_sach_top(unsigned char last_slot,int time_in ) {



  unsigned char i,nb_ul_sach[2]={0,0};

  unsigned short freq_alloc[2]={0,0},ul_sach_total_groups[2]={0,0},rach_total_groups=0;

  unsigned char ch_index,ch;

  first_sach_tx = 1;





  // first scan for all UL_SACH on TX to compute composite sch and get amplification factor

  for(i=0;i<NB_REQ_MAX;i++)
    if ((Macphy_req_table[0].Macphy_req_table_entry[i].Active)&&
        (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Direction == TX)) {
      if (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type == UL_SACH) {

	ch_index = Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.CH_index;
	if (ch_index < 2) {
	  freq_alloc[ch_index] |= Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc;
	  nb_ul_sach[ch_index]++;
	}
	else {

	}
      }
    }

  for (ch=0;ch<2;ch++) 
    if (freq_alloc[ch] != 0) {

      
      ul_sach_total_groups[ch] = phy_generate_sch(ch,
						  1+ch,
						  NUMBER_OF_GUARD_RACH_SYMBOLS+TX_RX_SWITCH_SYMBOL+ch,
						  freq_alloc[ch],
						  0,
						  NB_ANTENNAS_TX);
    }


  // Now rescan requests for this TTI and perform sach generation
  for(i=0;i<NB_REQ_MAX;i++)
    if ((Macphy_req_table[0].Macphy_req_table_entry[i].Active)&&
        (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Direction == TX)) {
      /*
   if (((mac_xface->frame/5) % 20) == 0)
      msg("[PHY][CODING] Frame %d: Req %d(%p), Active %d, Direction %d, Type %d\n",
	  mac_xface->frame,
	  i,
	  &Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req,
	  Macphy_req_table[0].Macphy_req_table_entry[i].Active,
	  Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Direction,
	  (Macphy_req_table[0].Macphy_req_table_entry[i].Active == 1)?
	  Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type:-1);
      */

      ch_index = Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.CH_index;

      switch (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type) {

      case RACH:

	if (last_slot == SLOTS_PER_FRAME - 1) {
	  // We're in the first slot of the TTI (also checked in phy_procedures.c)
	  //	  msg("[OPENAIR][PHY][CODING] frame %d: Generating RACH (pdu %p)\n",
	  //    mac_xface->frame,
	  //    Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.Rach_pdu.Rach_payload);
	  // Generate RACH pilot
	  rach_total_groups = phy_generate_sch(0,
					       0,
					       2+TX_RX_SWITCH_SYMBOL,
					       RACH0_FREQ_ALLOC,
					       0,
					       NB_ANTENNAS_TX);
	  // Coded Modulation front-end
	  phy_generate_sach1(0,  //stream_index
			     0,  //sacch_flag
			     SCH,//sch_type
			     0,  //sch_index
			     (unsigned char*)Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.Rach_pdu.Rach_payload,
			     NULL,
			     RACH_TIME_ALLOC,
			     RACH0_FREQ_ALLOC,
			     0,
			     NB_ANTENNAS_TX,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.tb_size_bytes,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Active_process_map,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.New_process_map,
			     1,
			     rach_total_groups);
	  // OFDM Modulation
	  phy_generate_sach2(0,
			     TX_RX_SWITCH_SYMBOL+3,
			     NUMBER_OF_RACH_SYMBOLS,
			     NB_ANTENNAS_TX);
	    
	    
	  Macphy_req_table[0].Macphy_req_cnt--;
	  Macphy_req_table[0].Macphy_req_table_entry[i].Active=0;
	  //	    Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Phy_Resources_Entry->Active=0;
	    

	}	
	break;
      

      case UL_SACH:


	if (last_slot == SLOTS_PER_FRAME - 1) {  
	  // We're in the first slot of the TTI (also checked in phy_procedures.c)
	  /*
	  if (ch_index == 1)	  	  
	    msg("[PHY][CODING] Frame %d, Time %d: Generating SACH, format %d, time %x, freq %x, coding %d, tb_size %d, num_tb %d, ap_map %x, np_map %x\n",
		mac_xface->frame,	      time_in,
		Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.format_flag,
		Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
		Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc,
		Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Coding_fmt,
		Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.tb_size_bytes,
		Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.num_tb,
		Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Active_process_map,
		Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.New_process_map);	      
	  */
	  // Generate the SACH coded-modulation front-end
	  	  
	  phy_generate_sach1(ch_index,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.format_flag,
			     SCH,
			     1+ch_index,
			     (unsigned char*)&Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.UL_sach_pdu.Sach_payload[0],
			     (unsigned char*)&Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.UL_sach_pdu.UL_sacch_pdu,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Coding_fmt,
			     NB_ANTENNAS_TX,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.tb_size_bytes,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Active_process_map,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.New_process_map,
			     first_sach_tx,
			     ul_sach_total_groups[ch_index]);
	  
	  if (first_sach_tx == 1) {
	    // If this is the first UL SACH in the period
	    // Clear first_sach_tx flag, note that this must be done after the generation

	    first_sach_tx = 0;
	  }

	  Macphy_req_table[0].Macphy_req_cnt--;
	  Macphy_req_table[0].Macphy_req_table_entry[i].Active=0;

	} 

	break;


    
      case DL_SACH:

	if (last_slot == SLOTS_PER_FRAME - 2) {
	  // We're in the last slot of the TTI (also checked in phy_procedures)
	  /*	  
	  msg("[PHY][CODING] Frame %d, Time %d: Generating SACH, format %d, time %x, freq %x, coding %d, tb_size %d, num_tb %d, ap_map %x, np_map %x\n",
	      mac_xface->frame,
	      time_in,
	      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.format_flag,
	      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
	      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc,
	      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Coding_fmt,
	      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.tb_size_bytes,
	      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.num_tb,
	      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Active_process_map,
	      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.New_process_map);	      
	  */
	  // Generate the SACH coded-modulation front-end
	  phy_generate_sach1(0,
			     0,        //sacch_flag
			     CHSCH,
			     1+ch_index,
			     (unsigned char*)&Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.DL_sach_pdu.Sach_payload[0],
			     NULL,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Coding_fmt,
			     NB_ANTENNAS_TX,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.tb_size_bytes,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Active_process_map,
			     Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.New_process_map,
			     first_sach_tx,
			     1);
	  
	  if (first_sach_tx == 1) {
	    // If this is the first DL SACH in the period, generate the pilot
	    /*
	    phy_generate_sch(0,FIRST_DL_SACH_SYMBOL
			     0xffff,
			     0,
			     NB_ANTENNAS_TX);
	    */

	    first_sach_tx = 0;
	  }

	

	  Macphy_req_table[0].Macphy_req_cnt--;
	  Macphy_req_table[0].Macphy_req_table_entry[i].Active=0;
	  //	  Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources_Entry->Active=0;
	}

      }  // DL_SACH
    } // Active Indication

  // Do second half of SACH modulation if a SACH was generated

  
  if (first_sach_tx==0) {  
    //There was a DL/UL SACH in this TTI
    if (mac_xface->is_cluster_head == 0)
      phy_generate_sach2(0,
			 FIRST_UL_SACH_SYMBOL,//first_symbol,
			 NUMBER_OF_UL_SACH_SYMBOLS,
			 NB_ANTENNAS_TX); //number_of_symbols);
    else {
      phy_generate_sach2(0,
			 FIRST_DL_SACH_SYMBOL,//first_symbol,
			 NUMBER_OF_DL_SACH_SYMBOLS,
			 NB_ANTENNAS_TX); //number_of_symbols);
    }
  }
  
}

#endif //OPENAIR2

static short qam64_table[6][8];

void generate_sach_64qam_table() {

  int a,b,c,index;


  for (a=-1;a<=1;a+=2) 
    for (b=-1;b<=1;b+=2) 
      for (c=-1;c<=1;c+=2) {
	index = (1+a)*2 + (1+b) + (1+c)/2;  
	qam64_table[0][index] = a*(QAM64_n1 + b*(QAM64_n2 + (c*QAM64_n3))); // 0 1 2
	
	qam64_table[1][index] = b*(QAM64_n1 + a*(QAM64_n2 + (c*QAM64_n3))); // 1 0 2
	
	qam64_table[2][index] = b*(QAM64_n1 + c*(QAM64_n2 + (a*QAM64_n3))); // 1 2 0
	
	qam64_table[3][index] = c*(QAM64_n1 + a*(QAM64_n2 + (b*QAM64_n3))); // 2 0 1
	
	qam64_table[4][index] = c*(QAM64_n1 + b*(QAM64_n2 + (a*QAM64_n3))); // 2 1 0
	
	qam64_table[5][index] = a*(QAM64_n1 + c*(QAM64_n2 + (b*QAM64_n3))); // 0 2 1
	//	printf("QAM64 i %d : %d\n",index,qam64_table[0][index]);
      } 
}


/*
		    switch (jj%6) {
		      

		    case 0: //0 1 2
		      
		      fft_input16[aa][off+((ii+aa)<<1)] = 
			((sach_data[jj]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset1]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj+qam64_offset2]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      fft_input16[aa][1+(off+((ii+aa)<<1))] = 
			((sach_data[jj+quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset1 + quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj+qam64_offset2 + quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      break;

		    case 1: //1 0 2
		      fft_input16[aa][off+((ii+aa)<<1)] = 
			((sach_data[jj+qam64_offset1]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj+qam64_offset2]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      fft_input16[aa][1+(off+((ii+aa)<<1))] = 
			((sach_data[jj+qam64_offset1 +quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj+qam64_offset2 + quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      break;

		    case 2: //1 2 0
		      fft_input16[aa][off+((ii+aa)<<1)] = 
			((sach_data[jj+qam64_offset1]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset2]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      fft_input16[aa][1+(off+((ii+aa)<<1))] = 
			((sach_data[jj+qam64_offset1+quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset2+quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj+quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      break;

		    case 3: // 2 0 1
		      fft_input16[aa][off+((ii+aa)<<1)] = 
			((sach_data[jj+qam64_offset2]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset1]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      fft_input16[aa][1+(off+((ii+aa)<<1))] = 
			((sach_data[jj+qam64_offset2 + quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset1 + quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj+ quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      break;

		    case 4: // 2 1 0
		      fft_input16[aa][off+((ii+aa)<<1)] = 
			((sach_data[jj+qam64_offset2]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset1]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      fft_input16[aa][1+(off+((ii+aa)<<1))] = 
			((sach_data[jj+qam64_offset2+quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset1 + quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj+quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      break;

		    case 5: //0 2 1
		      fft_input16[aa][off+((ii+aa)<<1)] = 
			((sach_data[jj]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset2]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj+qam64_offset1]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      fft_input16[aa][1+(off+((ii+aa)<<1))] = 
			((sach_data[jj+quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM1) : gain_lin_64QAM1) + 
			((sach_data[jj+qam64_offset2 + quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM2) : gain_lin_64QAM2) +
			((sach_data[jj+qam64_offset1 + quadrature_offset_sach]==0x80) ? (-gain_lin_64QAM3) : gain_lin_64QAM3);
		      break;

		    }
		    */

static int UL_GAIN[16] = {128,91,74,64,57,52,48,45,43,40,39,37,35,34,33,32};

void phy_generate_sach1(unsigned char stream_index,
			unsigned int sacch_flag,
			unsigned char sch_type,
			unsigned char sch_index,
			unsigned char *sach_pdu,
			unsigned char *sacch_pdu,
			unsigned char time_alloc,
			unsigned short freq_alloc,
			unsigned char coding_fmt,
			unsigned char nb_antennas_tx,
                        unsigned short tb_size_bytes,
                        unsigned int Active_process_map,
                        unsigned int New_process_map,
			unsigned char first_sach_flag,
			unsigned char total_groups) {

  unsigned int crc;
  int i,ii,j,jj,n,jjj,off,pilot_ind;


  unsigned char doing_sacch = 0;


  short gain_lin_QPSK,gain_lin_16QAM1,gain_lin_16QAM2,gain_lin_64QAM1,gain_lin_64QAM2,gain_lin_64QAM3;
  int *fft_input[nb_antennas_tx];
  short *fft_input16[nb_antennas_tx];


#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif // USER_MODE

  int first_symbol,number_of_symbols,first_symbol2;
  short number_of_groups;

  unsigned int sach_size_bytes,sach_size_bits;
  unsigned int sacch_size_bits;

  unsigned short quadrature_offset_sach,quadrature_offset_sacch;
  unsigned short qam16_offset;
  unsigned short qam64_offset1,qam64_offset2;

  unsigned char coding_rate;

  unsigned char aa,aa_offset;



  unsigned char num_tb,num_new_tb;

  unsigned char sach_data[(MAX_TB_SIZE_BYTES+4)*MAX_NUM_TB<<4];
  unsigned char *sacch_data = PHY_vars->sacch_data[0].encoded_data[0];

  unsigned short pos;


  int status;

  int qam64_table_offset_re = 0;
  int qam64_table_offset_im = 0;

  int reps=0;

  //#ifdef DEBUG_PHY

      



  //#endif // DEBUG_PHY
  // Encode data

  if (total_groups>NUMBER_OF_FREQUENCY_GROUPS) {

    msg("[PHY][SACH] sach.c: phy_generate_sach1, illegal number of total groups %d\n",total_groups);
#ifndef USER_MODE
    openair_sched_exit("");
#endif
    return;
  }

  if (stream_index>1) {
    msg("[PHY][SACH] Frame %d: Illegal stream_index %d\n",mac_xface->frame,stream_index);
#ifndef USER_MODE
    mac_xface->macphy_exit("");
#endif
    return;
  }

  if (coding_fmt > 1) {
    msg("[PHY][SACH] Frame %d: Illegal coding_fmt %d\n",mac_xface->frame,coding_fmt);
#ifndef USER_MODE
    mac_xface->macphy_exit("");
#endif
    return;
  }

  if (mac_xface->is_cluster_head == 0)
    first_symbol = TX_RX_SWITCH_SYMBOL + ((time_alloc >> 4)<<2);   // top nibble of time_alloc is first symbol in groups of 4 symbols
  else
    first_symbol = ((time_alloc >> 4)<<2);   // top nibble of time_alloc is first symbol in groups of 4 symbols


  number_of_groups = 0;

  if (mac_xface->is_cluster_head == 0)
    first_symbol2 = first_symbol - FIRST_UL_SACH_SYMBOL;
  else
    first_symbol2 = first_symbol - FIRST_DL_SACH_SYMBOL;

  if (time_alloc != RACH_TIME_ALLOC)
    number_of_symbols = (time_alloc & 0xf)<<2;
  else {
    number_of_symbols = NUMBER_OF_RACH_SYMBOLS;
    first_symbol      = TX_RX_SWITCH_SYMBOL+3;
    first_symbol2     = 0;
  }



  for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
    if ((freq_alloc & (1<<i)) > 0)  {// This freq group is allocated to the sach
      number_of_groups++;
    }
  }


  
  switch (coding_fmt&0x7) {

  case 0: // QPSK Single stream : Rate 1
    coding_rate = 1;
    break;
  case 1: // 16-QAM Single stream : Rate 2
  case 3: // QPSK Dual stream     : Rate 2
    coding_rate = 2;
    coding_rate = 2;
    break;
  case 2: // 64-QAM Single stream : Rate 3
    coding_rate = 3;
    break;
  case 4: // 16-QAM Dual stream : Rate 4
    coding_rate = 4;
    break;
  case 5: // 64-QAM Dual stream : Rate 6
    coding_rate = 6;
    break;
  default:
    coding_rate = 1;
    break;

  }

  sacch_size_bits = (sacch_flag == 0) ? 0 : SACCH_SIZE_BITS;

  // Number of coded bits on channel (after rate-matching)
  if (mac_xface->is_cluster_head == 0)
    sach_size_bits  = (((number_of_groups * NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP) * number_of_symbols)-sacch_size_bits)*coding_rate;
  else
    sach_size_bits  = (((number_of_groups * NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP) * number_of_symbols))*coding_rate;

  sach_size_bytes = sach_size_bits>>3;


  // scramble data



  /*   
  //  if (sch_index>1) {
    msg("[openair][PHY][CODING] Generate SACH1 (sch %d) stream_index %d(%x,%x) -> first_ul_sach_symbol %d, first_symbol %d, first_symbol2 %d, number_of_symbols %d, number_of_groups %d,number of sach bits %d, number of sacch bits %d,sach_pdu = %p\n",
	sch_index,
	stream_index,
	time_alloc,
	freq_alloc,
	FIRST_UL_SACH_SYMBOL,
	first_symbol,
	first_symbol2,
	number_of_symbols,
	number_of_groups,
	sach_size_bits,
	sacch_size_bits,
	sach_pdu);
    //  }  
    
    */

  // Compute number of new transport blocks for newly created HARQ processes
  num_new_tb=0;
  for (n=0;
       n<MAX_NUM_TB;
       n++) {

    if ((New_process_map & (1<<n)) != 0) {
  
      // Scramble new TB
      for (i=0;
	   i<tb_size_bytes;
	   i++) {
	PHY_vars->sach_data[0].tx_pdu[n][i] = (unsigned char)(sach_pdu[i+(tb_size_bytes*num_new_tb)] ^ scrambling_sequence[i]);
	//#ifdef DEBUG_PHY
	//	msg("%x:%x\n",i,sach_pdu[i]);
	//#endif //DEBUG_PHY
      }
      crc = crc24(&sach_pdu[(tb_size_bytes*num_new_tb)],
		  (tb_size_bytes)<<3);
      
      
      // Get crc
      *(unsigned int *)&PHY_vars->sach_data[0].tx_pdu[n][tb_size_bytes] = crc>>8;


#ifdef USER_MODE
#ifdef DEBUG_PHY
      msg("[PHY][CODING][SACH] TB %d TX CRC %x (sach size %d)\n",n,crc>>8,tb_size_bytes);
#endif //DEBUG_PHY
#endif //USER_MODE
      



      ccodedot11_encode(tb_size_bytes+4,
			PHY_vars->sach_data[0].tx_pdu[n],
			PHY_vars->sach_data[0].encoded_data[n],
			0);
      
      
#ifdef DEBUG_PHY
#ifdef USER_MODE
      if (first_sach_flag == 1) {
	sprintf(fname,"sach_encoded_output%d.m",n);
	sprintf(vname,"sach_encoded_out%d",n);
	write_output(fname,vname,
		     PHY_vars->sach_data[0].encoded_data[n],
		     (tb_size_bytes+4)<<4,
		     1,
		     4);
      }
      else {
	sprintf(fname,"sach_encoded_output%d2.m",n);
	sprintf(vname,"sach_encoded_out%d2",n);
	write_output(fname,vname,
		     PHY_vars->sach_data[0].encoded_data[n],
		     (tb_size_bytes+4)<<4,
		     1,
		     4);
      }
    
#endif //USER_MODE
#endif //DEBUG_PHY
      num_new_tb++;
    }    // round_indices[n] == 1 (new TB)
  }      // loop MAX number active TB


  /*
  msg("\nsach_size_bits = %d, sacch_size_bits = %d, bits_to_send = %d (num_tb=%d,tb_size_bytes %d)\n",
      sach_size_bits,
      sacch_size_bits,
      (num_tb*(tb_size_bytes+4))<<4,
      num_tb,
      tb_size_bytes);
  */
  // Copy active encoded HARQ processes to transmit buffer
  num_tb = 0;
  pos = 0;
  for (n=0;
       n<MAX_NUM_TB;
       n++) {
    if ((Active_process_map & (1<<n)) != 0) {  // this process is active in this TTI
      memcpy(&sach_data[pos],
	     PHY_vars->sach_data[0].encoded_data[n],
	     (tb_size_bytes+4)<<4);   // rate 1/2 code and 8bits/byte means multiply by 16! 

      pos+=(tb_size_bytes+4)<<4;
      num_tb++;
    }
  }
   

  status = rate_matching(2*sach_size_bits,
			(num_tb*(tb_size_bytes+4))<<4,
			&sach_data[0],
			2*coding_rate,
			0);  // offset to be dynamic later
#ifndef USER_MODE
  if (status == -1) {
    msg("[OPENAIR][PHY][SACH][STREAM/CH %d] sach.c: Rate_matching error during SACH encode, coding rate %d, freq_alloc %x, num_tb %d, AP %x, sach_size_bits %d, sacch_size_bits %d\n",
	stream_index,
	coding_rate,
	freq_alloc,
	num_tb,
	Active_process_map,
	sach_size_bits,
	sacch_size_bits);
    openair_sched_exit("[OPENAIR][PHY][SACH] sach.c: Rate_matching error during SACH encode");
  }
#endif

  // offset in samples between real and imaginary components of coded sequence (independent of modulation order)
  quadrature_offset_sacch = sacch_size_bits;
  quadrature_offset_sach  = (num_tb*(tb_size_bytes+4))<<3;

  // Do SACCH
  if (sacch_flag) {
    doing_sacch = 1;
    for (i=0;
	 i<SACCH_SIZE_BYTES-4;
	 i++) 
      PHY_vars->sacch_data[0].tx_pdu[0][i] = (unsigned char)(sacch_pdu[i] ^ scrambling_sequence[i]);
    
     
    crc = crc24(PHY_vars->sacch_data[0].tx_pdu[0],
		(SACCH_SIZE_BYTES-4)<<3);
    
    
    // Get crc
    *(unsigned int *)&PHY_vars->sacch_data[0].tx_pdu[0][SACCH_SIZE_BYTES-4] = crc>>8;
     
  
#ifdef USER_MODE
#ifdef DEBUG_PHY
    msg("[PHY][CODING][SACCH] TX CRC %x (sacch size %d)\n",crc>>8,SACCH_SIZE_BYTES);
#endif //DEBUG_PHY
#endif //USER_MODE
    

    ccodedot11_encode(SACCH_SIZE_BYTES,
		      PHY_vars->sacch_data[0].tx_pdu[0],
		      PHY_vars->sacch_data[0].encoded_data[0],
		      0);

    
#ifdef DEBUG_PHY
#ifdef USER_MODE
    if (first_sach_flag == 0)
      write_output("sacch_encoded_output.m","sacch_encoded_out",
		   PHY_vars->sacch_data[0].encoded_data[0],
		   2*sacch_size_bits,
		   1,
		   4);
    else
      write_output("sacch_encoded_output2.m","sacch_encoded_out2",
		   PHY_vars->sacch_data[0].encoded_data[0],
		   2*sacch_size_bits,
		   1,
		   4);
#endif //USER_MODE
#endif //DEBUG_PHY
  } // sacch_flag == 1



  // Set up FFT Input buffer
  for (i=0;i<nb_antennas_tx;i++) {
    fft_input[i]   = PHY_vars->sach_data[0].fft_input[i];
    fft_input16[i] = (short *)fft_input[i];
    // clear FFT input buffer (for zero carriers)
    if (first_sach_flag == 1) {
      Zero_Buffer((void *)fft_input[i],
		  NUMBER_OF_OFDM_CARRIERS_BYTES * 20); //number_of_symbols);
    }
  }

  
  if (mac_xface->is_cluster_head == 1) {
    gain_lin_QPSK = (short)((CHSCH_AMP*ONE_OVER_SQRT2_Q15*nb_antennas_tx)>>15);  
    //  printf("SACH: gain_lin_QPSK = %d\n",gain_lin_QPSK);
    
    switch (coding_fmt) {
    case 0:
      // QPSK single stream
      
      break;
    case 1:
      //16QAM Single stream
      gain_lin_16QAM1 = (short)(((int)CHSCH_AMP*QAM16_n1*nb_antennas_tx)>>15);
      gain_lin_16QAM2 = (short)(((int)CHSCH_AMP*QAM16_n2*nb_antennas_tx)>>15);
      
      break;
      
    case 2:
      //64QAM Single stream
      gain_lin_64QAM1 = (short)(((int)CHSCH_AMP*QAM64_n1*nb_antennas_tx)>>15);
      gain_lin_64QAM2 = (short)(((int)CHSCH_AMP*QAM64_n2*nb_antennas_tx)>>15);
      gain_lin_64QAM3 = (short)(((int)CHSCH_AMP*QAM64_n3*nb_antennas_tx)>>15);
      
      break;
    default:
      break;
    }

    // Fill in Pilots for DL_SACH
    if (first_sach_flag == 1) {
      for (n=first_symbol2;
	   n<first_symbol2+number_of_symbols;
	   n++) {
	pilot_ind = 0;
	off = n<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS); //offset of current symbol
	
	ii = FIRST_CARRIER_OFFSET; //offset of current subcarrier within symbol
	
	for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) { //increment here by frequency reuse factor 
	  
	  //	  ii = (FIRST_CARRIER_OFFSET + (i*NUMBER_OF_CARRIERS_PER_GROUP) + j)% NUMBER_OF_OFDM_CARRIERS;
	  //	printf("jj:%d,jjj:%d,ii:%d\n",jj,jjj,ii);
	  
	  for (aa=0;aa<nb_antennas_tx;aa++) {
	    if ((pilot_ind/NUMBER_OF_CHBCH_PILOTS)==(sch_index-1)) {
	      fft_input[aa][(off>>1)+ii+aa] = PHY_vars->chsch_data[sch_index].CHSCH_f_tx[aa][ii+aa]; 
	    }
	    else
	      fft_input[aa][(off>>1)+ii+aa] = 0; 
	    
	    pilot_ind++;
	  }
	  
	  ii=(ii+NUMBER_OF_CARRIERS_PER_GROUP);
	  if (ii>=NUMBER_OF_OFDM_CARRIERS)
	    ii-=NUMBER_OF_OFDM_CARRIERS;	
	} // groups (i)
      } // symbols
    } // first_sach_flag == 1
  } // is_cluster_head == 1/

  else {  // This is for UE
     
    gain_lin_QPSK = (short)((((int)SCH_AMP*UL_GAIN[total_groups-1]*(int)ONE_OVER_SQRT2_Q15))>>20)*nb_antennas_tx;  
    //    msg("SACH: gain_lin_QPSK = %d,total_groups %d,UL_GAIN %d,nb_antennas_tx %d\n",gain_lin_QPSK,total_groups,UL_GAIN[total_groups-1],nb_antennas_tx);
    
    switch (coding_fmt) {
    case 0:
      // QPSK single stream
      
      break;
    case 1:
      //16QAM Single stream

      gain_lin_16QAM1 = (short)((((int)SCH_AMP*UL_GAIN[total_groups-1]*QAM16_n1*nb_antennas_tx))>>20);
      gain_lin_16QAM2 = (short)((((int)SCH_AMP*UL_GAIN[total_groups-1]*QAM16_n2*nb_antennas_tx))>>20);
      
      break;
      
    case 2:
      //64QAM Single stream
      gain_lin_64QAM1 = (short)((((int)SCH_AMP*UL_GAIN[total_groups-1]*QAM64_n1*nb_antennas_tx))>>20);
      gain_lin_64QAM2 = (short)((((int)SCH_AMP*UL_GAIN[total_groups-1]*QAM64_n2*nb_antennas_tx))>>20);
      gain_lin_64QAM3 = (short)((((int)SCH_AMP*UL_GAIN[total_groups-1]*QAM64_n3*nb_antennas_tx))>>20);
      
      break;
    default:
      break;
    }
  }
  //  printf("Gain lin QPSK : %d\n",gain_lin_QPSK);
  //  printf("Gain lin 16-QAM : %d,%d\n",gain_lin_16QAM1,gain_lin_16QAM2);

  qam16_offset = quadrature_offset_sach>>1;
  qam64_offset1 = quadrature_offset_sach/3;
  qam64_offset2 = qam64_offset1<<1;
  
  reps=0;

  jj=0;
  jjj=0;
  for (n=first_symbol2;
       n<first_symbol2+number_of_symbols;
       n++) {


    off = n<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS);
    

    for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j+=nb_antennas_tx) {
      ii = FIRST_CARRIER_OFFSET+j;
      //      jjj = j%nb_antennas_tx;
      for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
	
	if ((freq_alloc & (1<<i)) > 0)  {// This freq group is allocated to the sach
	  
	  for (aa=0;aa<nb_antennas_tx;aa++) {

	    // Compute antenna offset for cycling
	    aa_offset = (aa+stream_index)%NB_ANTENNAS_TX;

	    // Check if this position is for pilots
	    if (j >= NUMBER_OF_SACH_PILOTS) {

	      if (doing_sacch == 1) {
		// SACCH USES QPSK Single Stream per CH by default

		fft_input16[aa][off+((ii+aa_offset)<<1)] = (sacch_data[jj]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
		fft_input16[aa][1+off+((ii+aa_offset)<<1)] = (sacch_data[jj+quadrature_offset_sacch]==0) ? (-gain_lin_QPSK) : gain_lin_QPSK;
		/*
		printf("sacch: aa %d : jj %d, ii %d j %d i %d off %d: (%d,%d)\n",aa_offset,jj, ii,j,i,off,
		     fft_input16[aa][off+((ii+aa)<<1)],
		     fft_input16[aa][1+off+((ii+aa)<<1)]);
		*/
		jj++;
	      }
	      else {

		//skip over punctured bits
		while ((sach_data[jj] & 0x80) == 0) {
		  jj++;
		}

		if (jj < quadrature_offset_sach) { // check that we haven't passed the end of the buffer
		
		  switch (coding_fmt) {
		  case 0:
		    // QPSK Single-Stream
		    fft_input16[aa][off+((ii+aa_offset)<<1)] = ((sach_data[jj]&0xbf)==0x80) ? (-gain_lin_QPSK) : gain_lin_QPSK;
		    fft_input16[aa][1+off+((ii+aa_offset)<<1)] = ((sach_data[jj+quadrature_offset_sach]&0xbf)==0x80) ? (-gain_lin_QPSK) : gain_lin_QPSK;
#ifdef DEBUG_PHY
#ifdef USER_MODE
		    
		    		    
		    msg("jj %d/%d %d (%x)(%d), off %d ii %d (%d,%d)\n",jj,quadrature_offset_sach,aa_offset,sach_data[jj],reps,off,ii,fft_input16[aa_offset][off+((ii+aa_offset)<<1)],fft_input16[aa_offset][1+off+((ii+aa_offset)<<1)]);
		    
#endif //USER_MODE
#endif //DEBUG_PHY
		    break;
		  case 1:
		    //16QAM Single-Stream
		    if (jj&1) {
		      if (((sach_data[jj]&0xbf)==0x80))
			fft_input16[aa_offset][off+((ii+aa_offset)<<1)] = 
			  -gain_lin_16QAM1 + (((sach_data[jj+qam16_offset]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
		      else
			fft_input16[aa_offset][off+((ii+aa_offset)<<1)] = 
			  gain_lin_16QAM1 + (((sach_data[jj+qam16_offset]&0xbf)==0x80) ? (-gain_lin_16QAM2) : gain_lin_16QAM2);
		      if (((sach_data[jj+quadrature_offset_sach]&0xbf)==0x80))
			fft_input16[aa_offset][1+(off+((ii+aa_offset)<<1))] = 
			  (-gain_lin_16QAM1) + (((sach_data[jj+qam16_offset + quadrature_offset_sach]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
		      else
			fft_input16[aa_offset][1+(off+((ii+aa_offset)<<1))] = 
			  (gain_lin_16QAM1) + (((sach_data[jj+qam16_offset + quadrature_offset_sach]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
		      
		    }
		    else {
		      if (((sach_data[jj+qam16_offset]&0xbf)==0x80))
			fft_input16[aa_offset][off+((ii+aa_offset)<<1)] = 
			  -gain_lin_16QAM1 + (((sach_data[jj]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
		      else
			fft_input16[aa_offset][off+((ii+aa_offset)<<1)] = 
			  gain_lin_16QAM1 + (((sach_data[jj]&0xbf)==0x80) ? (-gain_lin_16QAM2) : gain_lin_16QAM2);
		      if ((sach_data[jj+quadrature_offset_sach+qam16_offset]&0xbf)==0x80)
			fft_input16[aa_offset][1+(off+((ii+aa_offset)<<1))] = 
			  (-gain_lin_16QAM1) + (((sach_data[jj+quadrature_offset_sach]&0xbf)==0x80) ? (gain_lin_16QAM2) : (-gain_lin_16QAM2));
		      else
			fft_input16[aa_offset][1+(off+((ii+aa_offset)<<1))] = 
			  (gain_lin_16QAM1) + (((sach_data[jj+quadrature_offset_sach]&0xbf)==0x80) ? (-gain_lin_16QAM2) : (gain_lin_16QAM2));
		      /*	      printf("jj %d, ii %d : (%d,%d)\n",jj, ii,
				      fft_input16[j%nb_antennas_tx][off+((ii+aa_offset)<<1)],
				      fft_input16[j%nb_antennas_tx][1+off+((ii+aa_offset)<<1)]);*/
		      
		    }
		    /*
		    printf("aa_offset %d jj %d, ii %d (%d,%d,%d,%d)\n",aa_offset,jj,ii,
			   sach_data[jj],
			   sach_data[jj+qam16_offset],
			   sach_data[jj+quadrature_offset_sach],
			   sach_data[jj+quadrature_offset_sach+qam16_offset]);
		    */
		    break;
		  case 2:
		    //64QAM Single-Stream

		    qam64_table_offset_re = 0;
		    if ((sach_data[jj]&0xbf) == 0x81)
		      qam64_table_offset_re+=4;
		    if ((sach_data[jj+qam64_offset1]&0xbf) == 0x81)
		      qam64_table_offset_re+=2;
		    if ((sach_data[jj+qam64_offset2]&0xbf) == 0x81)
		      qam64_table_offset_re+=1;

		    qam64_table_offset_im = 0;
		    if ((sach_data[jj+quadrature_offset_sach]&0xbf) == 0x81)
		      qam64_table_offset_im+=4;
		    if ((sach_data[jj+qam64_offset1+quadrature_offset_sach]&0xbf) == 0x81)
		      qam64_table_offset_im+=2;
		    if ((sach_data[jj+qam64_offset2+quadrature_offset_sach]&0xbf) == 0x81)
		      qam64_table_offset_im+=1;

		    if (mac_xface->is_cluster_head == 0) {
		      fft_input16[aa_offset][off+((ii+aa_offset)<<1)]     = (short)((((int)SCH_AMP*UL_GAIN[total_groups-1]*qam64_table[jj%6][qam64_table_offset_re]*nb_antennas_tx))>>19);
		      fft_input16[aa_offset][1+(off+((ii+aa_offset)<<1))] = (short)((((int)SCH_AMP*UL_GAIN[total_groups-1]*qam64_table[jj%6][qam64_table_offset_im]*nb_antennas_tx))>>19);
		    }
		    else {
		      fft_input16[aa_offset][off+((ii+aa_offset)<<1)]     = (short)((((int)CHSCH_AMP*qam64_table[jj%6][qam64_table_offset_re]*nb_antennas_tx))>>14);
		      fft_input16[aa_offset][1+(off+((ii+aa_offset)<<1))] = (short)((((int)CHSCH_AMP*qam64_table[jj%6][qam64_table_offset_im]*nb_antennas_tx))>>14);

		    }
		    break;

		  case 3:
		    // QPSK Dual_stream
		    /*
		      fft_input16[j%nb_antennas_tx][off+((ii+aa)<<1)] = (sach_data[jj]==0) ? (-gain_lin) : gain_lin;
		      fft_input16[j%nb_antennas_tx][1+(off+((ii+aa)<<1))] = (sach_data[jj+quadrature_offset_sach]==0) ? (-gain_lin) : gain_lin;
		      // **************** Fix the antenna cycling for nb_antennas_tx>2 !!!! *********************88
		      fft_input16[(1+j)%nb_antennas_tx][off+((ii+aa)<<1)] = (sach_data[jj+QPSK_ds_offset]==0) ? (-gain_lin_QPSK_s2) : gain_lin_QPSK_s2;
		      fft_input16[(1+j)%nb_antennas_tx][1+(off+((ii+aa)<<1))] = (sach_data[jj+QPSK_ds_offset+quadrature_offset_sach]==0) ? (-gain_lin_QPSK_s2) : gain_lin_QPSK_s2;
		    */
		    break;
		  case 4:
		    //16QAM Dual-Stream
		    /*
		      fft_input16[j%nb_antennas_tx][off+((ii+aa)<<1)] = 
		      ((sach_data[jj+qam16_offset_s11]==0) ? (-gain_lin_16QAM_s11) : gain_lin_16QAM_s11) + 
		      ((sach_data[jj+qam16_offset_s12]==0) ? (-gain_lin_16QAM_s12) : gain_lin_16QAM_s12);
		      fft_input16[j%nb_antennas_tx][1+(off+((ii+aa)<<1))] = 
		      ((sach_data[jj+quadrature_offset_sach+qam16_offset_s11]==0) ? (-gain_lin_16QAM_s11) : gain_lin_16QAM_s11) + 
		      ((sach_data[jj+quadrature_offset_sach+qam16_offset_s12]==0) ? (-gain_lin_16QAM_s12) : gain_lin_16QAM_s12);
		      fft_input16[(1+j)%nb_antennas_tx][off+((ii+aa)<<1)] = 
		      ((sach_data[jj_qam16_offset_s21]==0) ? (-gain_lin_16QAM_s21) : gain_lin_16QAM_s21) + 
		      ((sach_data[jj+qam16_offset_s22]==0) ? (-gain_lin_16QAM_s22) : gain_lin_16QAM_s22);
		      fft_input16[(1+j)%nb_antennas_tx][1+(off+((ii+aa)<<1))] = 
		      ((sach_data[jj+qam16_offset_s21 + quadrature_offset_sach]==0) ? (-gain_lin_16QAM_s21) : gain_lin_16QAM_s21) + 
		      ((sach_data[jj+qam16_offset_s22 + quadrature_offset_sach]==0) ? (-gain_lin_16QAM_s22) : gain_lin_16QAM_s22);
		    */
		    break;
		  case 5:
		    break;
		  default:
		    break;
		  } // switch coding_fmt
		} // j < quadrature_offset_sach


		if ((sach_data[jj]  & 0x40) != 0){  // bit is to be repeated
		  sach_data[jj] &= 0xbf;           // clear repetition bit
		  reps++;
		}
		else 
		  jj++;                            // increment bit counter
		
	      } // Doing sacch
	    } // if j is not a pilot position 
	    else {  // This is for pilots


	      if (sch_type == SCH) {


		if (aa == 0) {
		  
#ifdef USER_MODE
#ifdef DEBUG_PHY
		
		  msg("SACH: stream %d, sch %d: Symbol %d Pilot (SCH) index %d : position %d, aa %d, -> %d\n",stream_index,sch_index,n,j,ii+stream_index,aa,
		      ((short*)PHY_vars->sch_data[sch_index].SCH_f_tx[0])[(ii+stream_index)<<1]);
		  
#endif
#endif
		  fft_input16[aa][off+((ii+stream_index)<<1)] = 
		    ((short*)PHY_vars->sch_data[sch_index].SCH_f_tx[0])[(ii+stream_index)<<1];
		  fft_input16[aa][1+off+((ii+stream_index)<<1)] = 
		    ((short*)PHY_vars->sch_data[sch_index].SCH_f_tx[0])[1+((ii+stream_index)<<1)];
		}
		else {
		  fft_input16[aa][off+((ii+aa)<<1)] = 0;
		  fft_input16[aa][1+off+((ii+aa)<<1)] = 0;
		}
	      } // SCH condition
	    }  // Pilot condition
	  } // antenna loop
	  
	  // if current index is the last of the sacch portion of codeword reset source bit counter to zero



	  if ((doing_sacch == 1) && (jj==(sacch_size_bits))) {
	    jj = 0;
	    doing_sacch = 0;
	  }
	} // freq_alloc test

	ii=(ii+NUMBER_OF_CARRIERS_PER_GROUP);
	if (ii>=NUMBER_OF_OFDM_CARRIERS)
	  ii-=NUMBER_OF_OFDM_CARRIERS;
      }// groups (i)
    }// carriers per group (j) 
  } // symbols (n)
  
  

#ifdef DEBUG_PHY
#ifdef USER_MODE  
  for (i=0;i<nb_antennas_tx;i++) {
    if (first_sach_flag==1) {
      sprintf(fname,"sach%d%d_data%d.m",0,i,sch_index-1);
      sprintf(vname,"sach%d%d_dat%d",0,i,sch_index-1);
    }
    else {
      sprintf(fname,"sach%d%d2_data%d.m",0,i,sch_index-1);
      sprintf(vname,"sach%d%d2_dat%d",0,i,sch_index-1);
    }
    write_output(fname,vname,
		 (short *)fft_input[i],
		 number_of_symbols*NUMBER_OF_OFDM_CARRIERS,
		 1,
		 1);
  }
#endif //USER_MODE
#endif //DEBUG_PHY  
  // to see a single tone at FS/4 at output
  //fft_input[64] = 16384;
  //fft_input[256+64] = 16384;
  //fft_input[512+64] = 16384;
  //fft_input[768+64] = 16384;
  //fft_input[1024+64] = 16384;
  //fft_input[1280+64] = 16384;
  //fft_input[1536+64] = 16384;
  
  //  msg("sample_offset %d\n",SAMPLE_OFFSET_SACH);
}

unsigned char phy_generate_sach2(unsigned char extension_switch, 
				 unsigned char first_symbol,
				 unsigned char number_of_symbols,
				 unsigned char nb_antennas_tx) {

  int i;
  int *fft_input[nb_antennas_tx];
  short *fft_input16[nb_antennas_tx];
  unsigned int sach_energy=0;
  unsigned char txp;

#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif // USER_MODE
  /*
  if (((mac_xface->frame/5) % 200) == 0)
    msg("[PHY][CODING] Frame %d: Generate SACH2 -> first_symbol %d, number_of_symbols %d\n",
	mac_xface->frame,
	first_symbol,
	number_of_symbols);
  */

  for (i=0;i<nb_antennas_tx;i++) {
    fft_input[i]   = PHY_vars->sach_data[0].fft_input[i];
    fft_input16[i] = (short *)fft_input[i];
    // clear FFT input buffer (for zero carriers)
  }
    
  for (i=0;i<nb_antennas_tx;i++) {

    PHY_ofdm_mod(fft_input[i],
		 (int *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[(extension_switch == 1) ? (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES) : (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)],   
		 LOG2_NUMBER_OF_OFDM_CARRIERS,
		 (unsigned char)number_of_symbols,
		 CYCLIC_PREFIX_LENGTH,
		 twiddle_ifft,
		 rev,
		 (extension_switch == 1) ? EXTENSION_TYPE : NONE
		 );

    sach_energy += signal_energy(&PHY_vars->tx_vars[i].TX_DMA_BUFFER[(extension_switch == 1) ? (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES) : (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)],
				 (number_of_symbols)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);

  }


#ifdef BIT8_TXMUX
  bit8_txmux(((extension_switch == 1) ? OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES : OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)*(number_of_symbols),
	     ((extension_switch == 1) ? (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES) : (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)));
#endif //BIT8_TXMUX






#ifdef DEBUG_PHY
#ifdef USER_MODE  
#ifndef BIT8_TXMUX
  for (i=0;i<nb_antennas_tx;i++) {
    sprintf(fname,"sach%d%d_sig.m",0,i);
    sprintf(vname,"sach%d%d",0,i);
    write_output(fname,vname,
		 (short *)&PHY_vars->tx_vars[i].TX_DMA_BUFFER[(extension_switch == 1) ? (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES) : (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)],
		 ((extension_switch == 1) ? OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES: OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)*(number_of_symbols),
		 1,
		 1);
  }
#else
    sprintf(fname,"sach_sig.m",0);
    sprintf(vname,"sach",0,i);
    write_output(fname,vname,
		 (short *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[(extension_switch == 1) ? (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES) : (first_symbol*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)],
		 ((extension_switch == 1) ? nb_antennas_tx*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES: nb_antennas_tx*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX)*(number_of_symbols),
		 1,
		 5);
#endif
#endif // USER_MODE
#endif // DEBUG_PHY			 



  txp = dB_fixed(sach_energy);
  /*
  if (((mac_xface->frame/5) % 200) == 0)
    msg("[PHY][CODING][SACH] frame %d: SACH TX power = %d\n",mac_xface->frame,txp);
  */
#ifdef BIT8_TXMUX
  txp-=BIT8_TX_SHIFT_DB;
#endif
  return(txp);
  
}


void phy_decode_sach_common(int first_symbol,
			    int number_of_symbols,
			    unsigned char nb_antennas_rx,
			    unsigned int sach_index) { 


  unsigned short i,aa;

  int *input;

  unsigned int rxp[nb_antennas_rx];
  unsigned int n0_energy[nb_antennas_rx];



#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif //USER_MODE
  int *rx_buffer[nb_antennas_rx];



  // Get RX_BUFFER pointers
  for (aa=0;aa<nb_antennas_rx;aa++) {
#ifdef HW_PREFIX_REMOVAL
    rx_buffer[aa] = (int *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[first_symbol<<LOG2_NUMBER_OF_OFDM_CARRIERS];
#else
    rx_buffer[aa] = (int *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(first_symbol<<LOG2_NUMBER_OF_OFDM_CARRIERS) + 
								(first_symbol)*CYCLIC_PREFIX_LENGTH];
#endif

    rxp[aa] = (unsigned int)dB_fixed(signal_energy(rx_buffer[aa],
						   OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES));

    n0_energy[aa] = 
      dB_fixed(signal_energy((int *)&PHY_vars->rx_vars[aa].RX_DMA_BUFFER[PHY_vars->rx_vars[0].offset+
									 CYCLIC_PREFIX_LENGTH + 
									 (TX_RX_SWITCH_SYMBOL) * OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
			     OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES));
 
  }

    
  /*
    msg("[openair][PHY][CODING] SACH Common Frame %d: SACH %d, first_symbol %d, number_of_symbols %d\n", 
	mac_xface->frame,
	sach_index,
	first_symbol,number_of_symbols);
    for (aa=0;aa<nb_antennas_rx;aa++)
      msg("[openair][PHY][CODING] SACH Common Frame %d: SACH %d, RX Energy %d dB, N0 %d dB\n",
	  mac_xface->frame,
	  sach_index,
	  rxp[aa],
	  n0_energy[aa]);
  */

  


 


  
  // This is the common part to all SACH in the period    
  for (aa = 0 ; aa < nb_antennas_rx ; aa++) {
    for (i=0;
	 i<number_of_symbols;
	 i++ ){
      
#ifdef HW_PREFIX_REMOVAL
      input = &rx_buffer[aa][(i<<LOG2_NUMBER_OF_OFDM_CARRIERS)];
#else
      input = &rx_buffer[aa][(i<<LOG2_NUMBER_OF_OFDM_CARRIERS) +
			     (i+1)*CYCLIC_PREFIX_LENGTH];
#endif //HW_PREFIX_REMOVAL




      //    msg("[openair][PHY][SACH %d] frame %d: Decoding -> FFT %d\n",0,frame,i);
      fft((short *)&input[0],
	  (short *)&PHY_vars->sach_data[sach_index].rx_sig_f[aa][i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)],
	  (short *)twiddle_fft,
	  rev,
	  LOG2_NUMBER_OF_OFDM_CARRIERS,
	  LOG2_NUMBER_OF_OFDM_CARRIERS/2,
	  0);
      


    }
    
    
#ifdef USER_MODE  
#ifdef DEBUG_PHY  


    sprintf(fname,"sach%d_rx_f%d.m",sach_index,aa);
    sprintf(vname,"sach%d_rxf%d",sach_index,aa);
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].rx_sig_f[aa][0],2*number_of_symbols*NUMBER_OF_OFDM_CARRIERS,2,1);

#endif // DEBUG_PHY
#endif // USER_MODE  
  }
}

//unsigned int first_rach = 0;
unsigned int first_sach_rx=0;

#ifdef OPENAIR2

void do_sach_stats(unsigned short Lchan_index,
		   unsigned short freq_alloc,
		   unsigned short number_of_symbols,
		   unsigned char sacch_flag,
		   unsigned char num_tb,
		   unsigned char tb_size_bytes,
		   unsigned char sach_index,
		   unsigned char nb_antennas_tx,
		   unsigned char nb_antennas_rx) {

  int nn,aa=0,i,number_of_active_groups=0,number_of_active_carriers,sacch_size_input_bits,sacch_size_encoded_bits;

  short *Isymb2[2];
  unsigned char User_index  = (Lchan_index>>3);
  unsigned char tchan_index = Lchan_index%8;
  
  if (mac_xface->is_cluster_head == 1)
    User_index--;

  if (tchan_index == 2)
    tchan_index = 0;
  if (tchan_index > 3)
    tchan_index -= 3;
  

  sacch_size_input_bits = (sacch_flag == 0) ? 0 : SACCH_SIZE_BITS;
  sacch_size_encoded_bits = sacch_size_input_bits<<1;
  

  
  
  for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++)
    if ((freq_alloc & (1<<i)) > 0)  // This freq group is allocated to the sach
      number_of_active_groups++;
  
  // Copy demodulated SACCH output
  
  
  number_of_active_carriers = number_of_active_groups * NUMBER_OF_CARRIERS_PER_GROUP*number_of_symbols;  
  
  nn=0;
  
  for (aa=0;aa<nb_antennas_rx;aa++)
    Isymb2[aa] = (short *)(&PHY_vars->sach_data[sach_index].rx_sig_f3[aa][0]);

  PHY_vars->Sach_diagnostics[User_index][tchan_index].active            = 1;
  PHY_vars->Sach_diagnostics[User_index][tchan_index].tb_size_bytes     = tb_size_bytes;
  PHY_vars->Sach_diagnostics[User_index][tchan_index].nb_tb             = num_tb;
  PHY_vars->Sach_diagnostics[User_index][tchan_index].nb_sacch_carriers = sacch_size_input_bits; 
  PHY_vars->Sach_diagnostics[User_index][tchan_index].nb_sach_carriers  = number_of_active_carriers - sacch_size_input_bits; 
  PHY_vars->Sach_diagnostics[User_index][tchan_index].freq_alloc = freq_alloc;   // loop through SACCH carriers first, skipping pilots

  /*    
  msg("[SACH][DEMOD] TTI %d: Saving sach_stats for User %d, Tchan %d, sacch_size %d, active_carriers %d\n",
      mac_xface->frame, User_index, tchan_index,sacch_size_input_bits,number_of_active_carriers);
  */

  for (i=0;i<sacch_size_input_bits<<1;i++) {
    
    //    if ((nn%(NUMBER_OF_CARRIERS_PER_GROUP<<1)) != 0 ) {
      PHY_vars->Sach_diagnostics[User_index][tchan_index].sacch_demod_data[i]  = 0;    // Complex output (8 bit per dim) 
      for (aa=0;aa<nb_antennas_rx;aa++)
	PHY_vars->Sach_diagnostics[User_index][tchan_index].sacch_demod_data[i]  += (Isymb2[aa][nn]/nb_antennas_rx);    // Complex output (8 bit per dim) 
      nn++;
      //    }
    //    else 
    //      nn+=4;//(NUMBER_OF_SACH_PILOTS<<1);
  }
  
  for (i=0;i<(number_of_active_carriers-sacch_size_input_bits)<<1;i++) {
    
    //    if ((nn%(NUMBER_OF_CARRIERS_PER_GROUP<<1)) != 0) {
      PHY_vars->Sach_diagnostics[User_index][tchan_index].sach_demod_data[i]     = 0;             // Complex output (8 bit per dim) 
      for (aa=0;aa<nb_antennas_rx;aa++)
	PHY_vars->Sach_diagnostics[User_index][tchan_index].sach_demod_data[i]  += (Isymb2[aa][nn]/nb_antennas_rx); // Complex output (8 bit per dim) 
      nn++;
      //    }
      //    else
      //      nn+=4;//(NUMBER_OF_SACH_PILOTS<<1);
    
  }
  

  
}


  
int phy_decode_sach_top(unsigned char last_slot) {

  MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry;
  int crc_status;
  unsigned char time_alloc;
  unsigned short freq_alloc;
  unsigned char i,j;
  unsigned char ch_index;

  /*
  if (((last_slot == 3) &&
       (mac_xface->is_cluster_head == 1)) ||
      
      (((last_slot == 1) &&
	(mac_xface->is_cluster_head == 0))))
  */  
  
  first_sach_rx = 1;


  // Find first symbol and number of symbols for common part of SACH

  for (i=0;i<NB_REQ_MAX;i++) {

    Macphy_data_req_entry = &Macphy_req_table[0].Macphy_req_table_entry[i];
    if ((Macphy_data_req_entry->Active == 1) &&
        (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Direction == RX)) {
      // RACH slot

      time_alloc = Macphy_data_req_entry->Macphy_data_req.Phy_resources->Time_alloc; 
      freq_alloc = Macphy_data_req_entry->Macphy_data_req.Phy_resources->Freq_alloc; 

      ch_index   = Macphy_data_req_entry->Macphy_data_req.CH_index;

      if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == RACH) {  // if this sach is a RACH

	if ( time_alloc == RACH_TIME_ALLOC) {  //verify integrity of time_alloc
	  if (last_slot == 3) { 

	    phy_channel_estimation_top(0,
				       2+TX_RX_SWITCH_SYMBOL,
#ifdef HW_PREFIX_REMOVAL
				       1
#else
				       0
#endif
				       ,0,
				       NB_ANTENNAS_RX,
				       SCH);

	    phy_decode_sach_common(3+TX_RX_SWITCH_SYMBOL,
				   NUMBER_OF_RACH_SYMBOLS,
				   NB_ANTENNAS_RX,
				   0);
	    
	    phy_decode_sach_2streams_ml(0,
					1,
					Macphy_data_req_entry->Macphy_data_req.Phy_resources,
					(unsigned char*)Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Rach_pdu->Rach_payload,
					NULL,
					NB_ANTENNAS_RX,
					NB_ANTENNAS_TXRX,
					0,  // sach_index
					0,  // sch_index
					0,  // stream_index
					Macphy_data_req_entry->Macphy_data_req.num_tb,
					Macphy_data_req_entry->Macphy_data_req.tb_size_bytes,
					Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Active_process_map,
					&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);
	    

#ifndef USER_MODE	    
	    if (Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0] == 0) {
	      openair_daq_vars.rach_detection_count++;
	      
	      msg("[PHY][CODING][RACH] frame %d: Rach detection count = %d, last status %d\n",
		  mac_xface->frame,
		  openair_daq_vars.rach_detection_count,
		  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);
	      
	    }
	    /*	      
	    if (((mac_xface->frame/5) % 200) == 0) {
	      msg("[PHY][CODING][RACH] frame %d: Rach detection count = %d, last status %d\n",
		  mac_xface->frame,
		  openair_daq_vars.rach_detection_count,
		  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);
	    }
	    */

#endif //USER_MODE	  
	      
	    
	    //	    msg("[PHY][CODING][SACH] MACPHY_req_table_entry.UL_SACH_PDU=%p,CRC status %d\n",
	    //		Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Rach_pdu,
	    //		Macphy_data_req_entry->Macphy_data_req.rx_status);
	    
	    mac_xface->macphy_data_ind(0,
				       &Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,
				       RACH,
				       0);
	   

	    /*
	      msg("[PHY][CODING][RACH] MACPHY_req_table_entry.RACH_PDU=%p,CRC status %d\n",
	      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Rach_pdu,
	      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);
	    */
	    Macphy_req_table[0].Macphy_req_cnt--;
	    Macphy_data_req_entry->Active=0;
	  
	  }
	}
	else {
	  msg("[PHY][CODING][RACH] Illegal RACH configuration Time alloc : %d(%d) Freq alloc: %d\n",time_alloc,RACH_TIME_ALLOC,freq_alloc);
#ifndef USER_MODE
	  openair_sched_exit("");
#endif
	}
      }
      else if ( (Macphy_data_req_entry->Macphy_data_req.Pdu_type == UL_SACH)) {
	// this is a SACH 
	if ( last_slot == 3) { // if this sach is in the last received slot
	  if (first_sach_rx==1) {
	    

	    phy_channel_estimation_top(0,NUMBER_OF_GUARD_RACH_SYMBOLS+TX_RX_SWITCH_SYMBOL+ch_index,
#ifdef HW_PREFIX_REMOVAL
					 1
#else
					 0
#endif
					 ,1+ch_index,
					 NB_ANTENNAS_RX,
					 SCH);
	    

	    // Do common SACH part here
	    phy_decode_sach_common(FIRST_UL_SACH_SYMBOL,
				   20,// 
				   NB_ANTENNAS_RX,
				   1);  


	  }

	  /*
	  	  msg("[PHY][DECODE][SACH] Frame %d: Calling decode_sach\n",mac_xface->frame);
	  */  
	  	  
	  phy_decode_sach_2streams_ml(Macphy_data_req_entry->Macphy_data_req.format_flag,
				      first_sach_rx,
				      Macphy_data_req_entry->Macphy_data_req.Phy_resources,
				      (unsigned char*)&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.UL_sach_pdu.Sach_payload[0],
				      (unsigned char*)&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.UL_sach_pdu.UL_sacch_pdu,
				      NB_ANTENNAS_RX,
				      NB_ANTENNAS_TXRX,
				      1,            // sach_index
				      1+ch_index,   // sch_index
				      ch_index,     // stream_index
				      Macphy_data_req_entry->Macphy_data_req.num_tb,
				      Macphy_data_req_entry->Macphy_data_req.tb_size_bytes,
				      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Active_process_map,
				      &Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);
	  
	  if (first_sach_rx == 1)
	    first_sach_rx = 0;

	  do_sach_stats(Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index,
			freq_alloc,
			NUMBER_OF_UL_SACH_SYMBOLS,
			Macphy_data_req_entry->Macphy_data_req.format_flag,
			Macphy_data_req_entry->Macphy_data_req.num_tb,
			Macphy_data_req_entry->Macphy_data_req.tb_size_bytes,
			1,
			NB_ANTENNAS_TXRX,
			NB_ANTENNAS_RX);
	  	
	  //	  if (((mac_xface->frame/5) % 200) == 0)
	  //	  if (ch_index==1)
	    /*
	    msg("[PHY][CODING][SACH] frame %d: UL_SACH (format %d) last_slot %d, time %x, freq %x, num_tb %d, tb_size %d, ap_map %x, Coding %d, PDU_type %d, crc[0] %d\n",
		mac_xface->frame,
		Macphy_data_req_entry->Macphy_data_req.format_flag,
		last_slot,
		time_alloc,
		freq_alloc,
		Macphy_data_req_entry->Macphy_data_req.num_tb,
		Macphy_data_req_entry->Macphy_data_req.tb_size_bytes,
		Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Active_process_map,
		Macphy_data_req_entry->Macphy_data_req.Phy_resources->Coding_fmt,
		//		Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.UL_sach_pdu.UL_sacch_pdu.Seq_index,
		Macphy_data_req_entry->Macphy_data_req.Pdu_type,
		Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);
	    */
	  



	  fill_sch_measurement_info(1+ch_index,
				    Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas,
				    freq_alloc);

	  /*
	  for (i=0;i<Macphy_data_req_entry->Macphy_data_req.num_tb;i++)
	    if (Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i] < 0)
	      break;
	  
	  if (i < Macphy_data_req_entry->Macphy_data_req.num_tb) {
	    msg("[PHY][SACH][DECODE] TTI %d (%d dBm, %d dBm): freq_alloc %x, Ntb %d :\n",
		mac_xface->frame,
		Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Wideband_rssi_dBm,
		Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Wideband_interference_level_dBm,
		freq_alloc,
		Macphy_data_req_entry->Macphy_data_req.num_tb);
	    for (i=0;i<Macphy_data_req_entry->Macphy_data_req.num_tb;i++)
	      msg("%d ",Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]);
	    msg("\n");

	    msg("[PHY][SACH][DECODE] Sub-bands SINR: ");

	    for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
	      if ((freq_alloc & (1<<i))!=0)
		msg("%d->%d dB ",i, PHY_vars->sch_data[1].subband_aggregate_sinr[i]);
	    }
	    msg("\n");
	  }
	  */
  	  	      
	  mac_xface->macphy_data_ind(0,
				     &Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,
				     UL_SACH,
				     (unsigned short)Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);
	  

	  Macphy_req_table[0].Macphy_req_cnt--;
	  Macphy_data_req_entry->Active=0;

	     
	}
      }
    
      else if ( (Macphy_data_req_entry->Macphy_data_req.Pdu_type == DL_SACH))
	{// this is a DL_SACH 
	  if ( last_slot == 1) { // if this sach is in the last received slot
	    if (first_sach_rx==1) {

	      // Do common SACH part here
	      phy_decode_sach_common(FIRST_DL_SACH_SYMBOL,
				     20,
				     NB_ANTENNAS_RX,
				     1);
 

	    }
	    	    	    
	    phy_decode_sach_2streams_ml(0,
					first_sach_rx,
					Macphy_data_req_entry->Macphy_data_req.Phy_resources,
					(unsigned char*)&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.DL_sach_pdu.Sach_payload[0],
					NULL,
					NB_ANTENNAS_RX,
					NB_ANTENNAS_TXRX,
					1, //sach_index
					1+Macphy_data_req_entry->Macphy_data_req.CH_index, //sch_index
					0,
					Macphy_data_req_entry->Macphy_data_req.num_tb,
					Macphy_data_req_entry->Macphy_data_req.tb_size_bytes,
					Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Active_process_map,
					&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);
	    

	    do_sach_stats(Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index,
			  freq_alloc,
			  NUMBER_OF_DL_SACH_SYMBOLS,
			  Macphy_data_req_entry->Macphy_data_req.format_flag,
			  Macphy_data_req_entry->Macphy_data_req.num_tb,
			  Macphy_data_req_entry->Macphy_data_req.tb_size_bytes,
			  1,
			  NB_ANTENNAS_TXRX,
			  NB_ANTENNAS_RX);
	    

	    if (first_sach_rx == 1)
	      first_sach_rx=0;

	    /*	    	  
	    for (i=0;i<Macphy_data_req_entry->Macphy_data_req.num_tb;i++)
	      if (Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i] < 0)
		break;

	    if (i < Macphy_data_req_entry->Macphy_data_req.num_tb) {
	      msg("[PHY][SACH][DECODE] TTI %d: freq_alloc %x, Ntb %d :\n",
		  mac_xface->frame,
		  freq_alloc,
		  Macphy_data_req_entry->Macphy_data_req.num_tb);
	      for (i=0;i<Macphy_data_req_entry->Macphy_data_req.num_tb;i++)
		msg("%d ",Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]);
	      msg("\n");
	    }
	    */
	    
	    //if (((mac_xface->frame) % 100) == 0)
	    /*
	    msg("[PHY][CODING][SACH][%d] frame %d: last_slot %d, LCHAN %d, num_tb %d, tb_size_bytes %d, freq_alloc %x, crc[0] %d\n",
		Macphy_data_req_entry->Macphy_data_req.CH_index,
		mac_xface->frame,
		last_slot,
		Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index,
		Macphy_data_req_entry->Macphy_data_req.num_tb,
		Macphy_data_req_entry->Macphy_data_req.tb_size_bytes,
		freq_alloc,
		Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);
	    */
	    
	    	      
	    mac_xface->macphy_data_ind(0,
				       &Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,
				       DL_SACH,
				       (unsigned short)Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);
	    
	    

	    Macphy_req_table[0].Macphy_req_cnt--;
	    Macphy_data_req_entry->Active=0;
	  }
	}
     
    } // If active request
  } // End Request loop
  return(crc_status);
}

#endif //OPENAIR2



#define LOG2_NUMBER_OF_FREQ_GROUPS 4


#ifndef EXPRESSMIMO_TARGET

__m64 perror64 __attribute__ ((aligned(16)));
__m64 Rsymb_conj64 __attribute__ ((aligned(16)));


// Channel Level Computation
  
void sach_channel_level(int sch_index,
			int aa,
			unsigned int *number_of_active_groups, 
			unsigned int *avg,
			unsigned short freq_alloc) {

  int chr,chi,i,j,ii;
  int *channel_matched_filter_f;

  *avg = 0;
  *number_of_active_groups = 0;
  // Compute average signal energy across frequency groups for detection

  if (mac_xface->is_cluster_head == 0)
    channel_matched_filter_f = PHY_vars->chsch_data[sch_index].channel_matched_filter_f[aa];
  else
    channel_matched_filter_f = PHY_vars->sch_data[sch_index].channel_matched_filter_f[aa];

  for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
    if ((freq_alloc & (1<<i)) > 0)  {// This freq group is allocated to the sach
      for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j++) {
	// This is the offset for the jth carrier in the ith group
	ii = (FIRST_CARRIER_OFFSET + (i*NUMBER_OF_CARRIERS_PER_GROUP) + j)% NUMBER_OF_OFDM_CARRIERS;
	
	
	chr = (unsigned int)((short*)channel_matched_filter_f)[0+(ii<<2)];  // real-part
	chi = (unsigned int)((short*)channel_matched_filter_f)[1+(ii<<2)];  // -imag-part
	*avg = *avg + chr*chr + chi*chi;
	//	  maxh = cmax(maxh,chr*chr + chi*chi); 
      }
      *number_of_active_groups = *number_of_active_groups + 1;
    }
  }
}

// ***************************************************            
// Phase error compensation      

static unsigned char log2ng[17] = {0,1,1,2,2,2,2,3,3,3,3,3,3,4,4,4,4};
int sach_phase_error_comp_ch(int sach_index,
			     int sch_index,
			     int perror_index,
			     int stream_index,
			     int aa,
			     int number_of_symbols,
			     int first_symbol,
			     unsigned short freq_alloc,
			     int nb_antennas_tx,
			     unsigned char do_rotate) {


  struct complex16 perror,*Rpilot,*Rsymb;
  __m64 *Rpilot64;
  
  register __m64 mm0,mm1;
  
  int ind,ind64;
  
  
  int i,aatx;
  short freq_group;
  unsigned char ng;
  int norm;
  
  if (NUMBER_OF_SACH_PILOTS == 0)
    norm=0; // exit properly
  // inner product of received SCH in pilot positions and received symbol

  //  printf("phase_comp_ch: stream %d,perror_index %d, sach_index %d, sch_index %d\n",stream_index,perror_index,sach_index,sch_index);

  for (i=0;i<number_of_symbols;i++) {
    if (NUMBER_OF_SACH_PILOTS>0) {


      Rsymb  = (struct complex16 *)&PHY_vars->sach_data[sach_index].rx_sig_f[aa][(first_symbol+i)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];

      Rpilot = (struct complex16 *)&PHY_vars->sch_data[sch_index].rx_sig_f[aa][0];

      Rpilot64 = (__m64 *)Rpilot;

      // clear phase average accumulator
      mm1 = _m_pxor(mm1,mm1);
      
      
      ind64 = FIRST_CARRIER_OFFSET;
      ind = ind64<<1;
      freq_group=0;
      ng=0;
      while (freq_group < NUMBER_OF_FREQUENCY_GROUPS) {
	//skip empty freq groups


	if (( (freq_alloc & (1<<freq_group))>0) && (freq_group!=8) ){ // the frequency group is allocated by MAC and is not the DC group, to avoid false estimation 
	  
	  ng++;

	  // This freq group is allocated to the sach
	  for (aatx=0;aatx<nb_antennas_tx;aatx++) {
	    if (aatx == stream_index) {

#ifdef DEBUG_PHY
#ifdef USER_MODE
	      
	      //	      if (sach_index > 0)
	      
	      //	    if (PHY_vars->PHY_measurements.rx_power_dB[sach_index][0] > 40)
	      
	      msg("[OPENAIR][PHY][SACH PHASE ERROR COMP] Power %d symbol %d (%p), pilot %d (%d), group %d: RX p (%d,%d), RX s (%d,%d)\n",
		  PHY_vars->PHY_measurements.rx_power_dB[sch_index][0],i,
		  Rsymb,
		  aatx,
		  ind/2,
		  freq_group,
		  Rpilot[ind].r,
		  Rpilot[ind].i,
		  Rsymb[ind].r,
		  Rsymb[ind].i);
	      
	      
#endif //USER_MODE
#endif //DEBUG_PHY
	    
	      ((short *)&Rsymb_conj64)[0] = Rsymb[ind].r;
	      ((short *)&Rsymb_conj64)[1] = Rsymb[ind].i;
	      ((short *)&Rsymb_conj64)[2] = -Rsymb[ind].i;
	      ((short *)&Rsymb_conj64)[3] = Rsymb[ind].r;
	    
	      mm0 = _mm_madd_pi16(Rpilot64[ind64],Rsymb_conj64);
	    
	      mm1 = _mm_add_pi32(mm0,mm1);
	    }	    
	    ind+=2;
	    ind64++;
	    
	  }
	  ind64+=(NUMBER_OF_CARRIERS_PER_GROUP-nb_antennas_tx);	  
	}
	else
	  ind64+=NUMBER_OF_CARRIERS_PER_GROUP;	  
	
	if (ind64>=NUMBER_OF_OFDM_CARRIERS)
	  ind64-=NUMBER_OF_OFDM_CARRIERS;
	ind=ind64<<1;
	
	
	freq_group++;
      }
      
      perror64 = _mm_srai_pi32(mm1,PERROR_SHIFT+log2ng[ng]);
      //      perror64 = _mm_srai_pi32(mm1,PERROR_SHIFT);
      perror.r = ((short *)&perror64)[0];  
      perror.i = ((short *)&perror64)[2];  
      
      norm = iSqrt((int)perror.r*perror.r + (int)perror.i*perror.i);

      // bring perror to unit circle with 8 bits of precision
      if (norm>0) {
	perror.r = (short)((((int)perror.r)<<5)/(int)norm);
	perror.i = (short)((((int)perror.i)<<5)/(int)norm);
      
      }

      PHY_vars->sach_data[perror_index].perror[aa][i] = perror;


#ifdef DEBUG_PHY
#ifdef USER_MODE
	
	//	if (PHY_vars->PHY_measurements.rx_power_dB[sach_index][0] > 40)
	    msg("[OPENAIR][PHY][SACH DEMOD] TTI %d : symbol %d, norm = %d, ind = %d, perror = (%d,%d) (%d,%d)\n",
		mac_xface->frame,
		i,
		norm,
		ind,
		perror.r,
		perror.i,
		((short *)&perror64)[0],  
		((short *)&perror64)[2]);
	    
#endif //USER_MODE
#endif //DEBUG_PHY
      if (do_rotate == 1) {


	ind = (FIRST_CARRIER_OFFSET<<1);
	freq_group=0;
	
	while (freq_group < NUMBER_OF_FREQUENCY_GROUPS) {
	  
	  if ((freq_alloc & (1<<freq_group))>0) { 
	    
	    rotate_cpx_vector2((short *)&Rsymb[ind],
			       (short *)&perror,
			       (short *)&Rsymb[ind],
			       NUMBER_OF_CARRIERS_PER_GROUP,
			       5,
			       0);//log2_perror_amp);
	    
	  
	

	  }
	  freq_group++;
	  ind+=(NUMBER_OF_CARRIERS_PER_GROUP<<1);
	  if (ind>=(NUMBER_OF_OFDM_CARRIERS<<1))
	    ind-=(NUMBER_OF_OFDM_CARRIERS<<1);	  
	  
	  
	  
	}
      }
      
    }
  }
  
  return(norm>=1);
}

static struct complex16 perrorv[32];
static int norm;

int sach_phase_error_comp_ue(unsigned char sach_index,
			     unsigned char sch_index,
			     unsigned char perror_index,
			     int aa,
			     int number_of_symbols,
			     int first_symbol,
			     unsigned short freq_alloc,
			     int nb_antennas_tx,
			     unsigned int first_sach_flag,
			     unsigned char do_rotate) {


  struct complex16 *Rpilot,*Rsymb;
  __m64 *Rpilot64;
  
  register __m64 mm0,mm1;
  
  int ind,ind64;
  
  
  int i,i2,aatx;
  short freq_group;
  unsigned char ng;

  
  if (NUMBER_OF_CHBCH_PILOTS == 0)
    norm=3; // exit properly
  // inner product of received SCH in pilot positions and received symbol
  for (i=0;i<number_of_symbols;i++) {
    if (NUMBER_OF_CHBCH_PILOTS>0) {
      
      //	perror.r = 0;
      //	perror.i = 0;
      
      Rsymb  = (struct complex16 *)&PHY_vars->sach_data[sach_index].rx_sig_f[aa][(first_symbol+i)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];

      Rpilot = (struct complex16 *)&PHY_vars->chsch_data[sch_index].rx_sig_f[aa][0];

      Rpilot64 = (__m64 *)Rpilot;

      // clear phase average accumulator
      mm1 = _m_pxor(mm1,mm1);
      
      
      ind64 = FIRST_CARRIER_OFFSET+(sch_index-1)*NUMBER_OF_CHBCH_PILOTS/2*NUMBER_OF_CARRIERS_PER_GROUP;
      ind = ind64<<1;

      if (first_sach_flag == 1) {
	for (i2=0;i2<NUMBER_OF_CHBCH_PILOTS;i2+=nb_antennas_tx) {
	  //for (i2=0;i2<32;i2+=nb_antennas_tx) {
	  if (ind != 0)  // skip DC carrier {
	    for (aatx=0;aatx<nb_antennas_tx;aatx++) {
#ifdef DEBUG_PHY
#ifdef USER_MODE
	      msg("[OPENAIR][PHY][SACH PHASE ERROR COMP] sach_index %d, sch_index %d, perror_index %d, symbol %d (%p), pilot %d (%d), RX p (%d,%d), RX s (%d,%d)\n",
		  sach_index,sch_index,perror_index,i,Rsymb,aatx,ind/2,Rpilot[ind].r,Rpilot[ind].i,
		  Rsymb[ind].r,Rsymb[ind].i);
	      
	      
#endif //USER_MODE
#endif //DEBUG_PHY
	      //	    p.r = ( ((Rpilot[ind].r*Rsymb[ind].r)>>PERROR_SHIFT) + ((Rpilot[ind].i*Rsymb[ind].i)>>PERROR_SHIFT) );
	      //	    p.i = ( ((Rpilot[ind].i*Rsymb[ind].r)>>PERROR_SHIFT) - ((Rpilot[ind].r*Rsymb[ind].i)>>PERROR_SHIFT) );
	      
	      //	    perror.r += p.r; 
	      //	    perror.i += p.i;
	      
	      ((short *)&Rsymb_conj64)[0] = Rsymb[ind].r;
	      ((short *)&Rsymb_conj64)[1] = Rsymb[ind].i;
	      ((short *)&Rsymb_conj64)[2] = -Rsymb[ind].i;
	      ((short *)&Rsymb_conj64)[3] = Rsymb[ind].r;
	      
	      mm0 = _mm_madd_pi16(Rpilot64[ind64],Rsymb_conj64);
	      
	      mm1 = _mm_add_pi32(mm0,mm1);
	      
	      ind+=2;
	      ind64++;
	    }
	  else  // skip DC carrier
	    ind64+=nb_antennas_tx;
	  
	  ind64+=(NUMBER_OF_CARRIERS_PER_GROUP-nb_antennas_tx);
	  if (ind64>=NUMBER_OF_OFDM_CARRIERS)
	    ind64-=NUMBER_OF_OFDM_CARRIERS;
	  ind=ind64<<1;
	  
	  
	}
	
	//	perror64 = _mm_srai_pi32(mm1,PERROR_SHIFT+LOG2_NUMBER_OF_CHBCH_PILOTS);
	perror64 = _mm_srai_pi32(mm1,PERROR_SHIFT+5);
	
	perrorv[i].r = ((short *)&perror64)[0];  
	perrorv[i].i = ((short *)&perror64)[2];  
	
	norm = iSqrt((int)perrorv[i].r*perrorv[i].r + (int)perrorv[i].i*perrorv[i].i);
	
	// bring perror to unit circle with 8 bits of precision
	if (norm>0) {
	  perrorv[i].r = (short)((((int)perrorv[i].r)<<5)/(int)norm);
	  perrorv[i].i = (short)((((int)perrorv[i].i)<<5)/(int)norm);
	}
	
	
	PHY_vars->sach_data[perror_index].perror[aa][i] = perrorv[i];
#ifdef DEBUG_PHY
#ifdef USER_MODE
	printf("aa %d, i %d -> perror[%d] (%d,%d)\n",aa,i,perror_index,
	       ((short *)&PHY_vars->sach_data[perror_index].perror[aa][i])[0],
	       ((short *)&PHY_vars->sach_data[perror_index].perror[aa][i])[1]);
	
#endif //USER_MODE
#endif //DEBUG_PHY
      }

      // Apply rotation to allocated frequencies

      if (do_rotate == 1) {
	ind = (FIRST_CARRIER_OFFSET<<1);
	freq_group=0;
	ng=0;
	while (freq_group < NUMBER_OF_FREQUENCY_GROUPS) {
	  
	  if ((freq_alloc & (1<<freq_group))>0) { 
	    
	    rotate_cpx_vector2((short *)&Rsymb[ind],
			       (short *)&perrorv[i],
			       (short *)&Rsymb[ind],
			       NUMBER_OF_CARRIERS_PER_GROUP,
			       5,
			       0);//log2_perror_amp);
	  }
	  
	  freq_group++;
	  ind+=(NUMBER_OF_CARRIERS_PER_GROUP<<1);
	  if (ind>=(NUMBER_OF_OFDM_CARRIERS<<1))
	    ind-=(NUMBER_OF_OFDM_CARRIERS<<1);	  
	}
	
#ifdef DEBUG_PHY
#ifdef USER_MODE
	
	if (mac_xface->is_cluster_head == 0)
	  msg("[OPENAIR][PHY][SACH DEMOD] TTI %d: symbol %d, norm = %d, perror = (%d,%d)\n",
	      mac_xface->frame,i,norm,perrorv[i].r,perrorv[i].i);
      
#endif //USER_MODE
#endif //DEBUG_PHY
      }
    }
  }

  return(norm>=1);
}


void sach_channel_compensation(int sach_index,
			       int sch_index,
			       int nb_antennas_rx,
			       short freq_alloc,
			       int log2_maxh,
			       unsigned char *I0_shift,
			       int number_of_symbols,
			       int first_symbol ) {

  int *channel_matched_filter_f,*channel_f,*mag_channel_f,aa,s,ind,freq_group;
  
  // Channel Compensation

  for (aa=0;aa<nb_antennas_rx;aa++){

    if (mac_xface->is_cluster_head == 0) {
      channel_matched_filter_f = PHY_vars->chsch_data[sch_index].channel_matched_filter_f[aa];
      channel_f                = PHY_vars->chsch_data[sch_index].channel_f[aa];
      mag_channel_f            = PHY_vars->chsch_data[sch_index].mag_channel_f[aa];
    }
    else {
      channel_matched_filter_f = PHY_vars->sch_data[sch_index].channel_matched_filter_f[aa];
      channel_f                = PHY_vars->sch_data[sch_index].channel_f[aa];
      mag_channel_f            = PHY_vars->sch_data[sch_index].mag_channel_f[aa];
    }

    // compute magnitude squared of channel

    ind = (FIRST_CARRIER_OFFSET<<1);
    freq_group=0;

    while (freq_group < NUMBER_OF_FREQUENCY_GROUPS) {
      
      if ((freq_alloc & (1<<freq_group))>0) { 

	mult_cpx_vector2((short *)&channel_matched_filter_f[ind],
			 (short *)&channel_f[ind],
			 (short *)&mag_channel_f[ind],
			 NUMBER_OF_CARRIERS_PER_GROUP,
			 log2_maxh+I0_shift[aa]);
      }
      freq_group++;
      ind+=(NUMBER_OF_CARRIERS_PER_GROUP<<1);
      if (ind>=(NUMBER_OF_OFDM_CARRIERS<<1))
	ind-=(NUMBER_OF_OFDM_CARRIERS<<1);	  
      
    }    
#ifdef USER_MODE
#ifdef DEBUG_PHY


#endif //DEBUG_PHY
#endif //USER_MODE

    for (s=0;
	 s<number_of_symbols;
	 s++ ){

      
#ifdef USER_MODE
#ifdef DEBUG_PHY
      msg("[openair][PHY][SACH %d] Compensating symbol %d (%d) shift %d (%d,%d)\n",sach_index,s,s+first_symbol,log2_maxh+I0_shift[aa],log2_maxh,I0_shift[aa]);
#endif //DEBUG_PHY
#endif //USER_MODE
      

      ind = (FIRST_CARRIER_OFFSET<<1);
      freq_group=0;
      
      while (freq_group < NUMBER_OF_FREQUENCY_GROUPS) {
	
	if ((freq_alloc & (1<<freq_group))>0) { 
	  
	  mult_cpx_vector2((short *)&PHY_vars->sach_data[sach_index].rx_sig_f[aa][ind+((first_symbol+s)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS))],
			   (short*)&channel_matched_filter_f[ind],
			   (short *)&PHY_vars->sach_data[sach_index].rx_sig_f2[aa][ind+((first_symbol+s)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS))],
			   NUMBER_OF_CARRIERS_PER_GROUP,
			   log2_maxh+I0_shift[aa]); 
	}
	freq_group++;
	ind+=(NUMBER_OF_CARRIERS_PER_GROUP<<1);
	if (ind>=(NUMBER_OF_OFDM_CARRIERS<<1))
	  ind-=(NUMBER_OF_OFDM_CARRIERS<<1);	  

      }

    }// symbols
  }// antennas
}

void sach_detection_stage0(int sach_index,
			   int sch_index,
			   int nb_antennas_rx,
			   int number_of_symbols,
			   int first_symbol,
			   unsigned short freq_alloc,
			   int number_of_active_carriers,
			   int *iii) {

  int aa,s,i,j,ii,jj;
  int *Csymb;
  short *temp_ptr;
  int *temp_ptr2,*pilot,temp_mag;

  register __m128i xmm0,xmm1,xmm2;
  __m128i *Csymb_128i[NB_ANTENNAS_RX],*Isymb,temp;; //*Cmag_128i[NB_ANTENNAS_RX],

  Isymb  = (__m128i *)PHY_vars->sach_data[sach_index].rx_sig_f4;

  for (aa=0;aa<nb_antennas_rx;aa++) {
    jj = 0; 
    if (mac_xface->is_cluster_head == 0) 
      pilot = PHY_vars->chsch_data[sch_index].mag_channel_f[aa];
    else
      pilot = PHY_vars->sch_data[sch_index].mag_channel_f[aa];

    for (s=0;
	 s<number_of_symbols;
	 s++ ){
      
      Csymb = (int *)&PHY_vars->sach_data[sach_index].rx_sig_f2[aa][(first_symbol+s)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS)];
      
      ii = FIRST_CARRIER_OFFSET;
      
      for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
	if ((freq_alloc & (1<<i)) > 0)  {// This freq group is allocated to the sach
	  
	  for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j++) {

	    
	    PHY_vars->sach_data[sach_index].rx_sig_f3[aa][jj]     = Csymb[ii<<1];
	    //	    if (s==0) {
	   
	    PHY_vars->sach_data[sach_index].mag_channel_f_16qam[aa][jj] = pilot[ii<<1];

	    //	    	    msg("**aa %d : jj %d \n",aa,jj,((short*)(&PHY_vars->sach_data[sach_index].mag_channel_f[aa][0]))[jj<<1],
	    //   ((short*)(PHY_vars->sach_data[sach_index].mag_channel_f[aa]))[1+(jj<<1)]);


	    jj=jj+1;
	    ii=ii+1;
	  } // carriers per group
	}
	else
	  ii=ii+NUMBER_OF_CARRIERS_PER_GROUP;
	
	if (ii==NUMBER_OF_OFDM_CARRIERS)
	  ii=0;
      }// groups 
    } //symbols

    // Set up amplitude vector for 16QAM detector
    for (i = 0 ; i<number_of_active_carriers; i++) {

      temp_ptr = (short *) &PHY_vars->sach_data[sach_index].mag_channel_f_16qam[aa][i];
      temp_mag = temp_ptr[0];

      //      msg("aa %d : i:%d - > %d %d\n",aa,i,temp_ptr[0],temp_ptr[1]);
      temp_ptr[0] = (temp_mag*QAM16_n1)>>15;


      temp_ptr[1] = temp_ptr[0];

      temp_ptr = (short *) &PHY_vars->sach_data[sach_index].mag_channel_f_64qama[aa][i];
      //      msg("aa %d : i:%d - > %d %d\n",aa,i,temp_ptr[0],temp_ptr[1]);
      temp_ptr[0] = (temp_mag*QAM64_n1)>>14;


      temp_ptr[1] = temp_ptr[0];

      temp_ptr = (short *) &PHY_vars->sach_data[sach_index].mag_channel_f_64qamb[aa][i];
      //      msg("aa %d : i:%d - > %d %d\n",aa,i,temp_ptr[0],temp_ptr[1]);
      temp_ptr[0] = (temp_mag*QAM64_n2)>>14;


      temp_ptr[1] = temp_ptr[0];
    }
    // Copy magnitude vector for 16qam portion
    for (s=1;s<number_of_symbols;s++) {
      temp_ptr2 = &PHY_vars->sach_data[sach_index].mag_channel_f_16qam[aa][s*number_of_active_carriers];

      for (i = 0 ; i<number_of_active_carriers; i++) {
	temp_ptr2[i] = PHY_vars->sach_data[sach_index].mag_channel_f_16qam[aa][i];
      }
      temp_ptr2 = &PHY_vars->sach_data[sach_index].mag_channel_f_64qama[aa][s*number_of_active_carriers];

      for (i = 0 ; i<number_of_active_carriers; i++) {
	temp_ptr2[i] = PHY_vars->sach_data[sach_index].mag_channel_f_64qama[aa][i];
      }
      temp_ptr2 = &PHY_vars->sach_data[sach_index].mag_channel_f_64qamb[aa][s*number_of_active_carriers];

      for (i = 0 ; i<number_of_active_carriers; i++) {
	temp_ptr2[i] = PHY_vars->sach_data[sach_index].mag_channel_f_64qamb[aa][i];
      }

    }
  } // rx antennas

  for (aa=0;aa<nb_antennas_rx;aa++) {
    Csymb_128i[aa] = (__m128i *)&PHY_vars->sach_data[sach_index].rx_sig_f3[aa][0];
    //    Cmag_128i[aa] = (__m128i *)&PHY_vars->sach_data[sach_index].mag_channel_f[aa][0];
  }
  
  ii = 0;   // Point to first carrier 
  

  switch (nb_antennas_rx) {
  case 1:
    xmm1 = _mm_set_epi32(0,0,0,0);
    break;
  case 2:
  case 3:
    
    
    
    xmm1 = _mm_set_epi32(0,0,0,1);
    
    break;
  case 4:
    xmm1 = _mm_set_epi32(0,0,0,2);
    break;
  default:
    xmm1 = _mm_set_epi32(0,0,0,1);
    break;
  }
  
  
  //  printf("***Rescaling jj=%d\n",jj);
  // NUMBER OF ALLOCATED CARRIERS (jj) MUST BE A MULTIPLE OF 8 TO WORK!!! ELSE FIX WRITES AT END OF LOOP
  for (i = 0 ; i<jj-1; i+=8) {
    

    xmm0 = _mm_xor_si128(xmm0,xmm0);
    xmm2 = _mm_xor_si128(xmm2,xmm2);
    //    xmm3 = _mm_xor_si128(xmm3,xmm3);
    //    xmm4 = _mm_xor_si128(xmm4,xmm4);
    
    
    // MR Combining
    //print_shorts(Csymb_128i[0][ii],"Csymb[0][ii]");
    //    print_shorts(Csymb_128i[0][ii+1],"Csymb[0][ii+1]");
    //    print_shorts(Csymb_128i[1][ii],"Csymb[1][ii]");
    //    print_shorts(Csymb_128i[1][ii+1],"Csymb[1][ii+1]");
    
    for (aa=0;aa<nb_antennas_rx;aa++){
      xmm0 = _mm_adds_epi16(xmm0,Csymb_128i[aa][ii]);
      xmm2 = _mm_adds_epi16(xmm2,Csymb_128i[aa][ii+1]);
    }
    xmm0 = _mm_sra_epi16(xmm0,xmm1);         // remove channel amplitude
    xmm2 = _mm_sra_epi16(xmm2,xmm1);         // remove channel amplitude
    
    //          temp = xmm0;
    //          print_shorts(temp,"xmm0(shift)=");
    //          temp = xmm2;
    //         print_shorts(temp,"xmm2(shift)=");
  
    Isymb[*iii] = _mm_packs_epi16(xmm0,xmm2);      // pack to 8 bits with saturation
    *iii = *iii + 1;

    //temp = Isymb[*iii-1];
    //                    print_bytes(temp,"xmm0(packs)=");
    
    ii+=2;
  } // carriers (i)

}

void sach_detection_stage0_ic(int sach_index,
			      int sch_index,
			      int number_of_symbols,
			      int first_symbol,
			      unsigned short freq_alloc,
			      int number_of_active_carriers,
			      int *iii) {

  int aa,s,i,j,ii,jj;
  int *Csymb;
  short *temp_ptr;
  int *temp_ptr2,*pilot,temp_mag;

  register __m128i xmm0,xmm1,xmm2;
  __m128i *Csymb_128i,*Isymb,temp;; //*Cmag_128i[NB_ANTENNAS_RX],

  Isymb  = (__m128i *)PHY_vars->sach_data[sach_index].rx_sig_f4;

  jj=0;

  for (s=0;
       s<number_of_symbols;
       s++ ){
    
    Csymb = (int *)&PHY_vars->sach_data[sch_index].rx_sig_f2[1][(first_symbol+s)<<(LOG2_NUMBER_OF_OFDM_CARRIERS)];
      
    ii = FIRST_CARRIER_OFFSET;

    for (i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
      if ((freq_alloc & (1<<i)) > 0)  {// This freq group is allocated to the sach
	
	for (j = 0 ; j<NUMBER_OF_CARRIERS_PER_GROUP; j++) {
	  

	  PHY_vars->sach_data[sach_index].rx_sig_f3[0][jj]     = Csymb[ii];
	  //	  	  printf("symbol %d, group %d, (%d,%d)\n",s,i,((short*)&Csymb[ii])[0],((short*)&Csymb[ii])[1]);
	  
	  
	  jj=jj+1;
	  ii=ii+1;
	} // carriers per group
      }
      else
	ii=ii+NUMBER_OF_CARRIERS_PER_GROUP;
      
      if (ii==NUMBER_OF_OFDM_CARRIERS)
	ii=0;
    }// groups 
  } //symbols
  

  Csymb_128i = (__m128i *)&PHY_vars->sach_data[sach_index].rx_sig_f3[0][0];

  
  ii = 0;   // Point to first carrier 
  
  
  //  printf("***Rescaling jj=%d\n",jj);
  // NUMBER OF ALLOCATED CARRIERS (jj) MUST BE A MULTIPLE OF 8 TO WORK!!! ELSE FIX WRITES AT END OF LOOP
  for (i = 0 ; i<jj-1; i+=8) {
    

    xmm0 = Csymb_128i[ii];
    xmm2 = Csymb_128i[ii+1];
    
  
    Isymb[*iii] = _mm_packs_epi16(xmm0,xmm2);      // pack to 8 bits with saturation
    *iii = *iii + 1;

    //temp = Isymb[*iii-1];
    //                    print_bytes(temp,"xmm0(packs)=");
    
    ii+=2;
  } // carriers (i)

}

void sacch_deinter(int sach_index,
		   int stream_index,
		   int sacch_size_encoded_bits,
		   int number_of_active_groups,
		   int nb_antennas_tx,
		   int *i,
		   int *j,
		   int *ii,
		   int *aa,
		   char **Isymb2) {


  char *dd;
  int quadrature_offset,nn;
  int aa_offset;

  dd = PHY_vars->sacch_data[sach_index].demod_data;
  
  
  quadrature_offset = sacch_size_encoded_bits>>1;
  
  //    for (n=0,nn=0;
  //	 n<quadrature_offset;
  //	 n++) {
  
  nn=0;
  while (nn<quadrature_offset) {

    aa_offset = (*aa + stream_index)%NB_ANTENNAS_TX;
    // QPSK detection (just keep real and imaginary parts
    if (*j >= NUMBER_OF_SACH_PILOTS) {
      dd[nn]                      = (*Isymb2)[(aa_offset+*ii)<<1]>>4;    // Real component
      dd[nn+quadrature_offset]    = (*Isymb2)[((aa_offset+*ii)<<1)+1]>>4;  // Imaginary components
      nn++;
    }
    
    *aa = *aa + 1; 
    if (*aa==nb_antennas_tx) {
      *aa=0;
      *ii=*ii+NUMBER_OF_CARRIERS_PER_GROUP;
      *i=*i+1;
      if (*i==number_of_active_groups){
	*i=0;
	*j=*j+nb_antennas_tx;
	if (*j==NUMBER_OF_CARRIERS_PER_GROUP) {
	  *j=0;
	  *Isymb2 = *Isymb2 + 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP; 

	}	
	*ii=*j;
      }  
    }
    
  }    
}



void sach16qam_detection(unsigned int sach_index,
			 unsigned int nb_antennas_rx,
			 unsigned int number_of_active_carriers_per_symbol,
			 unsigned int total_number_of_carriers,
			 unsigned int *iii) {

  int aa,aaa,l,ll,hii;
  __m128i *Csymb_128i[NB_ANTENNAS_RX],*Cmag_128i[NB_ANTENNAS_RX],*Isymb,temp;
  register __m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6;

  Isymb  = (__m128i *)PHY_vars->sach_data[sach_index].rx_sig_f4;

  for (aa=0;aa<nb_antennas_rx;aa++) {
    Csymb_128i[aa] = (__m128i *)&PHY_vars->sach_data[sach_index].rx_sig_f3[aa][0];
    Cmag_128i[aa] = (__m128i *)&PHY_vars->sach_data[sach_index].mag_channel_f_16qam[aa][0];
  }

  switch (nb_antennas_rx) {
  case 1:
    xmm1 = _mm_set_epi32(0,0,0,0);
    break;
  case 2:
  case 3:
    
    
    
    xmm1 = _mm_set_epi32(0,0,0,1);
    
    break;
  case 4:
    xmm1 = _mm_set_epi32(0,0,0,2);
    break;
  default:
    xmm1 = _mm_set_epi32(0,0,0,1);
    break;
  }
    
  xmm6 = _mm_xor_si128(xmm6,xmm6);
  ll = 0;   // Point to first carrier 
  hii=0;
  // we're doing some useless calculation here for SACCH carriers and pilots (to be improved)!!!!
  
  
  
  for (l = 0;     // first SACH carrier
       l<total_number_of_carriers;              // loop over total number of allocated carriers 
       l+=8) {                                  //8 at time (for 128-bit arith)
       
    
    //    msg("*******16QAM*********l=%d,ll=%d,iii=%d,hii=%d\n",l,ll,iii,hii);
    xmm0 = _mm_xor_si128(xmm0,xmm0);
    xmm2 = _mm_xor_si128(xmm2,xmm2);
    xmm3 = _mm_xor_si128(xmm3,xmm3);
    xmm4 = _mm_xor_si128(xmm4,xmm4);	
    
    // MR Combining
    for (aaa=0;aaa<nb_antennas_rx;aaa++){
      xmm0 = _mm_adds_epi16(xmm0,Csymb_128i[aaa][ll]);
      xmm2 = _mm_adds_epi16(xmm2,Csymb_128i[aaa][ll+1]);
      //      xmm3 = _mm_adds_epi16(xmm3,Cmag_128i[aaa][hii]);
      //      xmm4 = _mm_adds_epi16(xmm4,Cmag_128i[aaa][hii+1]);
      xmm3 = _mm_adds_epi16(xmm3,Cmag_128i[aaa][ll]);
      xmm4 = _mm_adds_epi16(xmm4,Cmag_128i[aaa][ll+1]);
    }

    /*
        temp = xmm0;
        print_shorts(temp,"mm0=");
        temp = xmm2;
        print_shorts(temp,"mm2=");
        temp = xmm3;
        print_shorts(temp,"mm3=");
        temp = xmm4;
        print_shorts(temp,"mm4=");
    */


    // take absolute values of statistics and subtract amplitude
    
    xmm5 = _mm_cmplt_epi16(xmm0,xmm6); //check for positions where <0
    
    
    xmm0 = _mm_xor_si128(xmm0,xmm5);   //negate negatives
    xmm0 = _mm_subs_epi16(xmm0,xmm3);

    xmm5 = _mm_cmplt_epi16(xmm2,xmm6); //check for positions where <0
    xmm2 = _mm_xor_si128(xmm2,xmm5);   //negate negatives
    xmm2 = _mm_subs_epi16(xmm2,xmm4);
    
    
    /* On SSSE3 machine with intrinsics defined can be done equivalently with PABSW as
       xmm0 = _mm_pabs_epi16(xmm0,xmm0);
       xmm2 = _mm_pabs_epi16(xmm0,xmm2);
       xmm0 = _mm_subs_epi16(xmm0,xmm3);
       xmm2 = _mm_subs_epi16(xmm2,xmm4);
    */
    
    xmm0 = _mm_sra_epi16(xmm0,xmm1);         // remove channel amplitude
    xmm2 = _mm_sra_epi16(xmm2,xmm1);         // remove channel amplitude
    
    Isymb[*iii] = _mm_packs_epi16(xmm0,xmm2);      // pack to 8 bits with saturation
    *iii=*iii + 1;
    ll+=2;


    /*
      temp = Isymb[*iii-1];

      printf("16qam iii %d\n",*iii-1);
      print_bytes(temp,"xmm0(packs)=");
    */
  } // carriers (i)
}


void sach64qam_detection(unsigned int sach_index,
			 unsigned int nb_antennas_rx,
			 unsigned int number_of_active_carriers_per_symbol,
			 unsigned int total_number_of_carriers,
			 unsigned int *iii) {

  int aa,aaa,l,ll,hii;
  __m128i *Csymb_128i[NB_ANTENNAS_RX],*Cmaga_128i[NB_ANTENNAS_RX],*Cmagb_128i[NB_ANTENNAS_RX],*Isymb,temp;
  register __m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
  __m128i zero;

  Isymb  = (__m128i *)PHY_vars->sach_data[sach_index].rx_sig_f4;

  for (aa=0;aa<nb_antennas_rx;aa++) {
    Csymb_128i[aa] = (__m128i *)&PHY_vars->sach_data[sach_index].rx_sig_f3[aa][0];
    Cmaga_128i[aa] = (__m128i *)&PHY_vars->sach_data[sach_index].mag_channel_f_64qama[aa][0];
    Cmagb_128i[aa] = (__m128i *)&PHY_vars->sach_data[sach_index].mag_channel_f_64qamb[aa][0];
  }

  switch (nb_antennas_rx) {
  case 1:
    xmm1 = _mm_set_epi32(0,0,0,0);
    break;
  case 2:
  case 3:
    
    
    
    xmm1 = _mm_set_epi32(0,0,0,1);
    break;
  case 4:
    xmm1 = _mm_set_epi32(0,0,0,2);
    break;
  default:
    xmm1 = _mm_set_epi32(0,0,0,1);
    break;
  }
    
  xmm6 = _mm_xor_si128(xmm6,xmm6);
  zero = xmm6;

  ll = 0;   //v b   sujm Point to first carrier 
  hii=0;
  // we're doing some useless calculation here for SACCH carriers and pilots (to be improved)!!!!
  
  
  
  for (l = 0;     // first SACH carrier
       l<total_number_of_carriers;              // loop over total number of allocated carriers 
       l+=8) {                                  //8 at time (for 128-bit arith)
       
    
    //    msg("*******16QAM*********l=%d,ll=%d,iii=%d,hii=%d\n",l,ll,iii,hii);
    xmm0 = _mm_xor_si128(xmm0,xmm0);
    xmm2 = _mm_xor_si128(xmm2,xmm2);
    xmm3 = _mm_xor_si128(xmm3,xmm3);
    xmm4 = _mm_xor_si128(xmm4,xmm4);	
    xmm6 = _mm_xor_si128(xmm6,xmm6);
    xmm7 = _mm_xor_si128(xmm7,xmm7);	
    
    // MR Combining
    for (aaa=0;aaa<nb_antennas_rx;aaa++){
      xmm0 = _mm_adds_epi16(xmm0,Csymb_128i[aaa][ll]);
      xmm2 = _mm_adds_epi16(xmm2,Csymb_128i[aaa][ll+1]);
      //      xmm3 = _mm_adds_epi16(xmm3,Cmag_128i[aaa][hii]);
      //      xmm4 = _mm_adds_epi16(xmm4,Cmag_128i[aaa][hii+1]);
      xmm3 = _mm_adds_epi16(xmm3,Cmaga_128i[aaa][ll]);
      xmm4 = _mm_adds_epi16(xmm4,Cmaga_128i[aaa][ll+1]);
      xmm6 = _mm_adds_epi16(xmm6,Cmagb_128i[aaa][ll]);
      xmm7 = _mm_adds_epi16(xmm7,Cmagb_128i[aaa][ll+1]);
    }

    /*    
        temp = xmm0;
        print_shorts(temp,"mm0=");
        temp = xmm2;
        print_shorts(temp,"mm2=");
        temp = xmm3;
        print_shorts(temp,"mm3=");
        temp = xmm4;
        print_shorts(temp,"mm4=");
        temp = xmm6;
        print_shorts(temp,"mm6=");
        temp = xmm7;
        print_shorts(temp,"mm7=");
    */

// take absolute values of statistics and subtract amplitude (first level)
    
    xmm5 = _mm_cmplt_epi16(xmm0,zero); //check for positions where <0
    xmm0 = _mm_xor_si128(xmm0,xmm5);   //negate negatives (|y1|)
    xmm0 = _mm_subs_epi16(xmm0,xmm3);  //subract scaled channel amp (|y1|-A1*h1)

    xmm5 = _mm_cmplt_epi16(xmm2,zero); //check for positions where <0
    xmm2 = _mm_xor_si128(xmm2,xmm5);   //negate negatives (|y2|)
    xmm2 = _mm_subs_epi16(xmm2,xmm4);  //subtract scaled channel amp (|y2|-A2*h2)

    // xmm0 now contains |y1|-A1*h1
    // xmm2 now contains |y2|-A1*h2

    Isymb[*iii] = _mm_packs_epi16(xmm0,xmm2);      // pack to 8 bits with saturation



    /*    
    printf("64qam iii %d\n",*iii);
    temp = xmm0;
    print_shorts(temp,"xmm0=");

    temp = xmm2;
    print_shorts(temp,"xmm2=");

    temp = Isymb[*iii];
    print_bytes(temp,"xmm0(packs)=");
    */

// Do second level
    

    xmm5 = _mm_cmplt_epi16(xmm0,zero); //check for positions where <0
    xmm0 = _mm_xor_si128(xmm0,xmm5);   //negate negatives
    xmm0 = _mm_subs_epi16(xmm0,xmm6);

    xmm5 = _mm_cmplt_epi16(xmm2,zero); //check for positions where <0
    xmm2 = _mm_xor_si128(xmm2,xmm5);   //negate negatives
    xmm2 = _mm_subs_epi16(xmm2,xmm7);


    // xmm0 now contains ||y|-A1*h|-A2*h
    
    // On SSSE3 machine with intrinsics defined can be done equivalently with PABSW as
    //   xmm0 = _mm_pabs_epi16(xmm0,xmm0);
    //   xmm2 = _mm_pabs_epi16(xmm0,xmm2);
    //   xmm0 = _mm_subs_epi16(xmm0,xmm3);
    //   xmm2 = _mm_subs_epi16(xmm2,xmm4);
    


    xmm0 = _mm_sra_epi16(xmm0,xmm1);         // remove channel amplitude
    xmm2 = _mm_sra_epi16(xmm2,xmm1);         // remove channel amplitude


    Isymb[(*iii)+(total_number_of_carriers>>3)] = _mm_packs_epi16(xmm0,xmm2);      // pack to 8 bits with saturation
    
    *iii=*iii + 1;
    ll+=2;
    
  } // carriers (i)
}



static unsigned char I0_compensation_table[11] = {0,0,1,1,1,2,2,2,3,3,3};

int phy_decode_sach(int sacch_flag,
		    unsigned int first_sach_flag,
		    PHY_RESOURCES *Phy_Resources_ptr,
		    unsigned char *Sach_payload,
		    unsigned char *Sacch_payload,
		    unsigned char nb_antennas_rx,
		    unsigned char nb_antennas_tx,
		    unsigned char sach_index,
		    unsigned char sch_index,
		    unsigned char stream_index,
		    unsigned char num_tb,
		    unsigned short tb_size_bytes,
		    unsigned int active_processes,
		    int *crc_status) { 
  
  int i,ii,first_symbol,j=0,n,aa,iii,nn,ret;
  int aa_offset;
  unsigned int oldcrc,crc;
  
  unsigned int avg;
  unsigned int i2;
  
  unsigned char log2_maxh;

  int sacch_valid = 1;

  unsigned char coding_fmt;

  unsigned char *Sach_payload2;

  unsigned char *sach_pdu,*sacch_pdu;
  char *dd;
  unsigned int sach_size_encoded_bits,sach_size_bits,sach_size_bytes,sacch_size_input_bits,sacch_size_encoded_bits;
  
#ifdef USER_MODE
  char fname[40],vname[40];
#endif //USER_MODE
  unsigned char time_alloc,coding_rate;
  unsigned short freq_alloc,number_of_symbols;


  unsigned int number_of_active_groups;
  unsigned int number_of_active_carriers;



  char *Isymb2,*Isymb2_16qam,*Isymb2_64qama,*Isymb2_64qamb;
  __m128i *Isymb;


  unsigned int quadrature_offset,qam16_offset,qam16_offset2,qam64_offseta,qam64_offsetb,qam64_offset,qam64_offset2a,qam64_offset2b;      

  int status;

  unsigned char I0_shift[NB_ANTENNAS_RX];
  char I0_min=0,I0_argmin=0,I0_diff,I0_sch_index=0;
  unsigned char rep=0;
  short tmp_re1,tmp_im1,tmp_re2,tmp_im2;

  // check parameters
  if (!Phy_Resources_ptr) {
    msg("[PHY][DECODE SACH] Undefined PHY_Resources, exiting\n");
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);

  }

  if (num_tb>MAX_NUM_TB) {
    msg("[PHY][DECODE SACH] TTI %d: Illegal number of TB, exiting (freq %x, NTb %d)\n",
	mac_xface->frame,
	Phy_Resources_ptr->Freq_alloc,
	num_tb);
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);
  }

  if (tb_size_bytes>MAX_TB_SIZE_BYTES) {
    msg("[PHY][DECODE SACH] Illegal number of TB, exiting\n");
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);
  }

  if ((sacch_flag == 1) && (Sacch_payload == NULL)) {
    msg("[PHY][DECODE SACH] Sacch_payload undefined, exiting\n");
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);
  }
    
  if (Sach_payload == NULL) {
    msg("[PHY][DECODE SACH] Sach_payload undefined, exiting\n");
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);
    
  }

  time_alloc = Phy_Resources_ptr->Time_alloc;
  freq_alloc = Phy_Resources_ptr->Freq_alloc;
  coding_fmt = Phy_Resources_ptr->Coding_fmt;

  if (mac_xface->is_cluster_head == 1) {
    first_symbol = (int)(((time_alloc >> 4)<<2) - (NUMBER_OF_UL_CONTROL_SYMBOLS));   // top nibble of time_alloc is first symbol in groups of 4 symbols
  }
  else 
    first_symbol = (int)(((time_alloc >> 4)<<2) - FIRST_DL_SACH_SYMBOL);   // top nibble of time_alloc is first symbol in groups of 4 symbols

  
  if (time_alloc == RACH_TIME_ALLOC) {
    number_of_symbols = NUMBER_OF_RACH_SYMBOLS;
    first_symbol=0;
  }
  else {
    number_of_symbols = (unsigned short)((time_alloc & 0xf)<<2);
    // Do a check of SACH parameters to avoid crashing in case of CHBCH_PDU error
    if ( (first_symbol < 0) || 
	 (number_of_symbols >= 64) ) {
      msg("[openair][PHY][CODING] Frame %d: SACH parameter error : first_symbol = %d, number_of_symbols= %d\n",
	  mac_xface->frame,first_symbol,number_of_symbols);
      if (sacch_flag == 1)
	crc_status[0] = -SACH_ERROR;
      else
	crc_status[0] = -SACCH_ERROR;
      return(0);
    }
  }
  
  
#ifdef DEBUG_PHY
  //  if (((mac_xface->frame) % 100) == 0)
    msg("[openair][PHY][CODING] Frame %d: Decode SACH %d, SCH %d (S+N0=%d  N0=%d): Sacch_flag %d, first_sach_flag %d, first_symbol %d, length %d, time_alloc %x, freq_alloc %x, Sach_payload %p\n",
	mac_xface->frame,
	sach_index,
	sch_index,
	PHY_vars->PHY_measurements.rx_power_dB[sch_index][0],
	PHY_vars->PHY_measurements.n0_power_dB[sch_index][0],
	sacch_flag,
	first_sach_flag,
	(int)first_symbol,
	number_of_symbols,
	time_alloc,
	freq_alloc,
	Sach_payload);
  
  
#endif // DEBUG_PHY 
  
  
    
    // Compute I0 mismatch between receive antennas
    // I0_min and I0_argmin contain the interference level and index of the antenna with the weakest interference
    
    I0_sch_index = (mac_xface->is_cluster_head == 0) ? 0 : 1;
    
    I0_min = PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][0];
    I0_argmin = 0;
    
      
    for (aa=1;aa<nb_antennas_rx;aa++){
      if (I0_min > PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa]) {
	I0_argmin = aa;
	I0_min =  PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa];
      }    
    }
    
    for (aa=0;aa<nb_antennas_rx;aa++) {
      I0_diff = PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa] - I0_min;
      if (I0_diff > 10)
	I0_diff=10;
      I0_shift[aa] = I0_compensation_table[I0_diff];
      //      printf("aa %d: IO_shift %d (%d dB)\n",aa,I0_shift[aa],PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa]);
    }

    /*    
    //  if ((mac_xface->frame % 100) == 0) {
        msg("[OPENAIR][SACH] TTI %d: ",mac_xface->frame);
    for (aa=0;aa<nb_antennas_rx;aa++)
      msg("I0(%d)=%d dBm, I0_shift(%d)=%d  ",aa,PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa],aa,I0_shift[aa]);
    msg("\n");
    //  }
  
    */


  //  for (aa=0;aa<nb_antennas_rx;aa++) {

  aa =I0_argmin;

  // Compute the average signal level on the antenna with lowest inteference/noise level
  sach_channel_level(sch_index,
		     aa,
		     &number_of_active_groups,
		     &avg,
		     (unsigned short) freq_alloc);


  for (aa=0;aa<nb_antennas_rx;aa++) {
    if (mac_xface->is_cluster_head == 0)
      ret = sach_phase_error_comp_ue(sach_index,
				     sch_index,
				     sch_index,
				     aa,
				     number_of_symbols,
				     first_symbol,
				     freq_alloc,
				     nb_antennas_tx,
				     first_sach_flag,
				     1);
    else
      ret = sach_phase_error_comp_ch(sach_index,
				     sch_index,
				     sch_index,
				     stream_index,
				     aa,
				     number_of_symbols,
				     first_symbol,
				     freq_alloc,
				     nb_antennas_tx,
				     1);
	



  }

  if (!ret) {
    
#ifdef DEBUG_PHY      
    if (sach_index >0 )
      msg("[OPENAIR][PHY][CODING][SACH] Frame: %d Abandoning sach: signal too weak \n",mac_xface->frame); 
#endif //DEBUG_PHY
    
    // Abandon decoding, signal is too weak
    if (sacch_flag == 1) 
      crc_status[0]=-SACH_MISSING;
    else
      crc_status[0]=-SACH_MISSING;
    
    return(0);
  }

  number_of_active_carriers = number_of_active_groups * NUMBER_OF_CARRIERS_PER_GROUP;  
  // find maximum bit position of I/Q components for rescaling
  avg/=(number_of_active_carriers);
  
  
  
  log2_maxh = (log2_approx(avg)/2);// + 12 - 15;//(log2_approx(avg))-8;
  
  
  sacch_size_input_bits = (sacch_flag == 0) ? 0 : SACCH_SIZE_BITS;
  sacch_size_encoded_bits = sacch_size_input_bits<<1;
  
  
  
#ifdef USER_MODE
#ifdef DEBUG_PHY
  if (sach_index == 1)
    msg("[openair][PHY][SACH %d] log2_maxh %d\n",sach_index,log2_maxh);
#endif //DEBUG_PHY
#endif //USER_MODE

    


  sach_channel_compensation(sach_index,
			    sch_index,
			    nb_antennas_rx,
			    freq_alloc,
			    log2_maxh,
			    &I0_shift[0],
			    number_of_symbols,
			    first_symbol);




  //  msg("log2_maxh = %d\n",log2_maxh);


  // QPSK/first stage 16QAM portion of Detection, SACCH+SACH
  iii=0;
  sach_detection_stage0(sach_index,
			sch_index,
			nb_antennas_rx,
			number_of_symbols,
			first_symbol,
			freq_alloc,
			number_of_active_carriers,
			&iii);


  // SAVE Internal Signals for OCTAVE
#ifdef USER_MODE  
#ifdef DEBUG_PHY  
  for (aa=0;aa<nb_antennas_rx;aa++) {

    if (first_sach_flag == 1) {
      sprintf(fname,"sach_comp_output%d.m",aa);
      sprintf(vname,"sach_comp_out%d",aa);
    }
    else {
      sprintf(fname,"sach_comp_output%d2.m",aa);
      sprintf(vname,"sach_comp_out%d2",aa);
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sch_index].rx_sig_f2[aa][0],
		 2*NUMBER_OF_OFDM_CARRIERS*number_of_symbols,  // length
		 2,   // decimation 1
		 1);  // complex 16-bit R/I
    if (first_sach_flag == 1) {
      sprintf(fname,"sach_demod_output%d.m",aa);
      sprintf(vname,"sach_demod_out%d",aa);
    }
    else {
      sprintf(fname,"sach_demod_output%d2.m",aa);
      sprintf(vname,"sach_demod_out%d2",aa);
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].rx_sig_f3[aa][0],
		 number_of_active_carriers*number_of_symbols,  // length
		 1,   // decimation 1
		 1);  // complex 16-bit R/I

    if (first_sach_flag == 1) {
      sprintf(fname,"sach_magh_16qam_output%d.m",aa);
      sprintf(vname,"sach_magh_16qam_out%d",aa);
    }
    else {
      sprintf(fname,"sach_magh_16qam_output%d2.m",aa);
      sprintf(vname,"sach_magh_16qam_out%d2",aa);
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].mag_channel_f_16qam[aa][0],
		 number_of_active_carriers*number_of_symbols,  // length
		 1,   // decimation 1
		 1);  // complex 16-bit R/I

    if (first_sach_flag == 1) {
      sprintf(fname,"sach_magh_64qama_output%d.m",aa);
      sprintf(vname,"sach_magh_64qama_out%d",aa);
    }
    else {
      sprintf(fname,"sach_magh_64qama_output%d2.m",aa);
      sprintf(vname,"sach_magh_64qama_out%d2",aa);
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].mag_channel_f_64qama[aa][0],
		 number_of_active_carriers*number_of_symbols,  // length
		 1,   // decimation 1
		 1);  // complex 16-bit R/I

    if (first_sach_flag == 1) {
      sprintf(fname,"sach_magh_64qamb_output%d.m",aa);
      sprintf(vname,"sach_magh_64qamb_out%d",aa);
    }
    else {
      sprintf(fname,"sach_magh_64qamb_output%d2.m",aa);
      sprintf(vname,"sach_magh_64qamb_out%d2",aa);
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].mag_channel_f_64qamb[aa][0],
		 number_of_active_carriers*number_of_symbols,  // length
		 1,   // decimation 1
		 1);  // complex 16-bit R/I

  }
#endif // DEBUG_PHY
#endif // USER_MODE  
  
 
  

  // SACCH Deinterleaving + Data detection (QPSK)


  //  off = 0;
  j=0;
  i=0;

  Isymb = (__m128i *)&PHY_vars->sach_data[sach_index].rx_sig_f4[0];
  Isymb2 = (char*)PHY_vars->sach_data[sach_index].rx_sig_f4;

  ii=0;
  aa=0;




  //
  // **********SACCH Decoding*****************
  //

  if (sacch_flag==1) {

    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY][CODING][SACCH] sacch_size %d, sacch_size bytes %d, freq_alloc %x, time_alloc %x, number_of_active_groups %d, number_of_symbols %d, NUMBER_OF_CARRIERS_PER_GROUP %d\n",SACCH_SIZE_BITS,SACCH_SIZE_BYTES,
	  freq_alloc,time_alloc,
	  number_of_active_groups,number_of_symbols,NUMBER_OF_CARRIERS_PER_GROUP); 
    */
    //    printf("Isymb2 (before sacch) %p\n",Isymb2);
    sacch_deinter(sach_index,stream_index,sacch_size_encoded_bits,
		  number_of_active_groups,nb_antennas_tx,
		  &i,&j,&ii,&aa,&Isymb2);    
    
    

    // Keep ii, i, j and aa alive for SACH below!!!!
    
    
#ifdef USER_MODE  
#ifdef DEBUG_PHY
    if (first_sach_flag == 1) 
      write_output("sacch_decode_input.m","sacch_decode_in",
		   &PHY_vars->sacch_data[sach_index].demod_data[0],
		   SACCH_SIZE_BITS*2,1,4);
    else
      write_output("sacch_decode_input2.m","sacch_decode_in2",
		   &PHY_vars->sacch_data[sach_index].demod_data[0],
		   SACCH_SIZE_BITS*2,1,4);

#endif // DEBUG_PHY
#endif // USER_MODE  
    

    
    // Viterbi Decoding
    
    
    sacch_pdu  = PHY_vars->sacch_data[sach_index].demod_pdu;

    
    Zero_Buffer(sacch_pdu,
		SACCH_SIZE_BYTES+8);  // +8 to guarantee a multiple of 8 bytes
     
    
    
    phy_viterbi_dot11_sse2(PHY_vars->sacch_data[sach_index].demod_data,
			   sacch_pdu,
			   SACCH_SIZE_BITS);
    
    oldcrc= *((unsigned int *)(&sacch_pdu[SACCH_SIZE_BYTES-4]));
    oldcrc&=0x00ffffff;
    
    crc = crc24(sacch_pdu,
		(SACCH_SIZE_BYTES-4)<<3)>>8;
    
    
    
    
    
#ifdef DEBUG_PHY    
    msg("Received CRC : %x\n",oldcrc);
    msg("Computed CRC : %x\n",crc);
#endif // DEBUG_PHY
    
    // descramble data
    
    for (n=0;
	 n<SACCH_SIZE_BYTES-4;
	 n++) {
      sacch_pdu[n] = sacch_pdu[n] ^ scrambling_sequence[n];
      Sacch_payload[n] = sacch_pdu[n];
#ifdef DEBUG_PHY
            msg("%x ",sacch_pdu[n]);
#endif //DEBUG_PHY
    }
#ifdef DEBUG_PHY
         msg("\n");
#endif //DEBUG_PHY

    // Get SACCH Information
    
    if (crc == oldcrc) {  // SACCH CRC passes get coding format for SACH data detection

      //      coding_fmt = ((UL_SACCH_PDU*)Sacch_payload)->Coding_fmt;
      //      coding_fmt = 0;
    }
    else  // SACCH CRC didn't pass 
      sacch_valid = 0;

    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY][CODING][SACCH] Frame %d: sacch_status %d\n",mac_xface->frame,
	  sacch_valid);
    */


  }  // sacch_flag == 1


  if (sacch_valid) {

    switch (coding_fmt&0x7) {

    case 0: // QPSK Single stream : Rate 1
      coding_rate = 1;
      break;
    case 1: // 16-QAM Single stream : Rate 2
    case 3: // QPSK Dual stream     : Rate 2
      coding_rate = 2;
      break;
    case 2: // 64-QAM Single stream : Rate 3
      coding_rate = 3;
      break;
    case 4: // 16-QAM Dual stream : Rate 4
      coding_rate = 4;
      break;
    case 5: // 64-QAM Dual stream : Rate 6
      coding_rate = 6;
      break;
    default:
      coding_rate = 1;
      break;
      
    }

    // sach size after rate matching
    if (mac_xface->is_cluster_head == 1)
      sach_size_bits = (((number_of_active_groups * NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP)*number_of_symbols)- sacch_size_input_bits)*coding_rate<<1; 
    else
      sach_size_bits = (((number_of_active_groups * NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP)*number_of_symbols))*coding_rate<<1; 

    sach_size_encoded_bits = (num_tb * (tb_size_bytes+4))<<4; 

    /*
      if (((mac_xface->frame/5) % 200) == 0)
      if (sach_index == 1)
      msg("[OPENAIR][PHY][CODING][SACH] sach_size_encoded_bits %d, sach_size_bits %d, sacch_size_input_bits %d, tb_size bytes %d, freq_alloc %x, time_alloc %x, coding fmt %d, number_of_active_groups %d, number_of_symbols %d, NUMBER_OF_CARRIERS_PER_GROUP %d, NUM Pilots %d\n",
      sach_size_encoded_bits,
      sach_size_bits,
      sacch_size_input_bits,
      tb_size_bytes,
      freq_alloc,
      time_alloc,
      coding_fmt,
      number_of_active_groups,
      number_of_symbols,
      NUMBER_OF_CARRIERS_PER_GROUP,
      NUMBER_OF_SACH_PILOTS); 
    */
    

    // second stage 16QAM detection
    if ((coding_fmt&0x7) == 1) {  // do second stream for 16-QAM




      sach16qam_detection(sach_index,
			  nb_antennas_rx,
			  number_of_active_carriers,
			  number_of_active_carriers*number_of_symbols,
			  &iii);
      
    }
    else if ((coding_fmt&0x7) == 2) {

      sach64qam_detection(sach_index,
			  nb_antennas_rx,
			  number_of_active_carriers,
			  number_of_active_carriers*number_of_symbols,
			  &iii);
    }
    


    // SACH Deinterleaving + Data detection (QPSK)
    // retrieving aa, ii, i, and j from above


    dd = PHY_vars->sach_data[sach_index].demod_data;
    
    quadrature_offset = sach_size_encoded_bits>>1;


    // Clear LLR Buffer
    Zero_Buffer(dd,
		sach_size_encoded_bits+8);

    // Tag non-punctured bits
    status = rate_matching(sach_size_bits,
			   sach_size_encoded_bits,
			   dd,
			   2*coding_rate,
			   0);  // offset to be dynamic later


#ifndef USER_MODE
    if (status == -1) {
      msg("[PHY][SACH] sach.c: Rate matching error during SACH decode, freq %x, numtb %d, num_groups %d tb_size_bytes %d\n",
	  freq_alloc,num_tb,number_of_active_groups,tb_size_bytes);

      openair_sched_exit("[PHY][SACH] sach.c: Rate matching error during SACH decode");
      return(0);
    }
#endif 

#ifdef USER_MODE  
#ifdef DEBUG_PHY  

    //    printf("rx_sig_f4 = %p\n",PHY_vars->sach_data[sach_index].rx_sig_f4);
    if (first_sach_flag == 1) {
      sprintf(fname,"sach_rescale_output0.m");
      sprintf(vname,"sach_rescale_out0");
    }
    else {
      sprintf(fname,"sach_rescale_output02.m");
      sprintf(vname,"sach_rescale_out02");
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].rx_sig_f4[0],
		 number_of_active_carriers*number_of_symbols*coding_rate*2,  // length
		 1,   // decimation 1
		 4);  // real 8-bit
    
#endif // DEBUG_PHY
#endif // USER_MODE  


    //    printf("Isymb2 (before sach) %d\n",Isymb2 - (char*)PHY_vars->sach_data[sach_index].rx_sig_f4);
    switch (coding_fmt&7) {
      case 0 :   // QPSK
	
	//      for (n=0,nn=0;
	//	   n<quadrature_offset;
	//	   n++) {
	nn=0;
	while (nn<quadrature_offset) {

	  aa_offset = (aa+stream_index)%NB_ANTENNAS_TX;
	  if (j>=NUMBER_OF_SACH_PILOTS) {
	    
	    // skip and null out punctured bitss
	    while ((dd[nn]&0x80) == 0) {
	      
	      dd[nn] = 0;
	      dd[nn+quadrature_offset] = 0;
	      //	      	    printf("nn %d punctured\n",nn);  
	      nn++;
	    }
	    //	    printf("dd[%d] = %x ->",nn,dd[nn]);
	    if (nn<quadrature_offset) {
	      if (rep == 0) {
		if (dd[nn] == (char)0x80) {
		  dd[nn]                    = Isymb2[((ii+aa_offset)<<1)]>>4;    // Real component
		  dd[nn+quadrature_offset]  = Isymb2[((ii+aa_offset)<<1)+1]>>4;  // Imaginary components
		  //		  printf("nn %d, ii %d, j %d (%d,%d)\n",nn,ii,j,dd[nn],dd[nn+quadrature_offset]); 
		  nn++;
		}
		else {  // store first repeated bit
		  tmp_re1 = Isymb2[((ii+aa_offset)<<1)];
		  tmp_im1 = Isymb2[((ii+aa_offset)<<1)+1];
		  //		  printf("(rep) nn %d, ii %d, j %d (%d,%d)\n",nn,ii,j,tmp_re1,tmp_im1); 
		  rep = 1;
		}
	      }
	      else { // Do MR combining of repeated bit
		tmp_re1 += Isymb2[((ii+aa_offset)<<1)];
		if (tmp_re1 > 127)
		  tmp_re1 = 127;
		else if (tmp_re1 < -128)
		  tmp_re1 = -128;
		tmp_im1 += Isymb2[((ii+aa_offset)<<1)+1];
		if (tmp_im1 > 127)
		  tmp_im1 = 127;
		else if (tmp_im1 < -128)
		  tmp_im1 = -128;

		dd[nn]                    = tmp_re1>>4;    // Real component
		dd[nn+quadrature_offset]  = tmp_im1>>4;  // Imaginary components
		  //	    printf("nn %d, ii %d, j %d (%d,%d)\n",nn,ii,j,dd[nn],dd[nn+quadrature_offset]); 
		nn++;
		rep=0;
	      }
	    }
	  }
	  aa++;
	  if (aa==nb_antennas_tx) {
	    aa=0;
	    ii+=NUMBER_OF_CARRIERS_PER_GROUP;
	    i++;
	    if (i==number_of_active_groups){
	      i=0;
	      j+=nb_antennas_tx;
	      if (j==NUMBER_OF_CARRIERS_PER_GROUP) {
		j=0;
		Isymb2 += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      }	
	      ii=j;
	    }  
	  }
	} // nn<quadrature_offset  
	break;

    case 1: // coding_fmt != 0 // 16-QAM
      qam16_offset      = quadrature_offset>>1;
      qam16_offset2     = qam16_offset + quadrature_offset;
      
      Isymb2_16qam      = Isymb2+ 2*number_of_active_groups * number_of_symbols * NUMBER_OF_CARRIERS_PER_GROUP;
      
      //      for (n=0,nn=0;
      //	   n<qam16_offset;
      //	   n++) {
      nn=0;
      while (nn<qam16_offset) {
	 
	if (j>=NUMBER_OF_SACH_PILOTS) {
	  //	  printf("nn %d\n",nn);
	  
	  // skip and null out punctured bitss
	  while ((dd[nn]&0x80) == 0) {
	    //	    	    printf("%d,%d,%d,%d punctured\n",nn,nn+quadrature_offset,nn+qam16_offset,nn+qam16_offset2);
	    dd[nn] = 0;
	    dd[nn+quadrature_offset] = 0;
	    dd[nn+qam16_offset] = 0;
	    dd[nn+qam16_offset2] = 0;
	    nn++;
	  }

	  if (nn<qam16_offset) { // not past the end yet

	    if (rep == 0) {
	      //	      printf("dd[%d] = %x\n",nn,(unsigned char)dd[nn]);
	      if (dd[nn]==(char)0x80) { // bit is not punctured and not repeated
		if (nn&1) { // bit deinterleaving
		  
		  dd[nn]                    = Isymb2[((ii+aa)<<1)]>>4;    // Real component
		  dd[nn+qam16_offset]       = Isymb2_16qam[((ii+aa)<<1)]>>4;    // Real component
		  dd[nn+quadrature_offset]  = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
		  dd[nn+qam16_offset2]      = Isymb2_16qam[((ii+aa)<<1)+1]>>4;  // Imaginary components

		  //		  printf("nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,dd[nn],dd[nn+qam16_offset],dd[nn+quadrature_offset],dd[nn+qam16_offset2]); 
		}
		else {
		  
		  dd[nn+qam16_offset]       = Isymb2[((ii+aa)<<1)]>>4;    // Real component
		  dd[nn]                    = Isymb2_16qam[((ii+aa)<<1)]>>4;    // Real component
		  dd[nn+qam16_offset2]      = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
		  dd[nn+quadrature_offset]  = Isymb2_16qam[((ii+aa)<<1)+1]>>4;  // Imaginary components

		  //		  printf("nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,dd[nn],dd[nn+qam16_offset],dd[nn+quadrature_offset],dd[nn+qam16_offset2]); 
		}
		nn++;
	      }
	      else {   // bit is not punctured but repeated
		
		if (nn&1) { // bit deinterleaving
		  
		  tmp_re1                   = Isymb2[((ii+aa)<<1)];    // Real component
		  tmp_re2                   = Isymb2_16qam[((ii+aa)<<1)];    // Real component
		  tmp_im1                   = Isymb2[((ii+aa)<<1)+1];  // Imaginary components
		  tmp_im2                   = Isymb2_16qam[((ii+aa)<<1)+1];  // Imaginary components
		  //		  printf("(rep0) nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,tmp_re1,tmp_re2,tmp_im1,tmp_im2); 
		}
		else {
		  
		  tmp_re2                   = Isymb2[((ii+aa)<<1)];    // Real component
		  tmp_re1                   = Isymb2_16qam[((ii+aa)<<1)];    // Real component
		  tmp_im2                   = Isymb2[((ii+aa)<<1)+1];  // Imaginary components
		  tmp_im1                   = Isymb2_16qam[((ii+aa)<<1)+1];  // Imaginary components
		  //		  printf("(rep0) nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,tmp_re1,tmp_re2,tmp_im1,tmp_im2); 
		}


		rep = 1;
		
	      }
	    }
	    else {   //rep = 1

	      if (nn&1) { // bit deinterleaving
		
		tmp_re1                   += Isymb2[((ii+aa)<<1)];    // Real component
		tmp_re2                   += Isymb2_16qam[((ii+aa)<<1)];    // Real component
		tmp_im1                   += Isymb2[((ii+aa)<<1)+1];  // Imaginary components
		tmp_im2                   += Isymb2_16qam[((ii+aa)<<1)+1];  // Imaginary components
		//		printf("(rep 1)nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,tmp_re1,tmp_re2,tmp_im1,tmp_im2); 
	      }
	      else {
		
		tmp_re2                   += Isymb2[((ii+aa)<<1)];    // Real component
		tmp_re1                   += Isymb2_16qam[((ii+aa)<<1)];    // Real component
		tmp_im2                   += Isymb2[((ii+aa)<<1)+1];  // Imaginary components
		tmp_im1                   += Isymb2_16qam[((ii+aa)<<1)+1];  // Imaginary component
		//		printf("(rep 1)nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,tmp_re1,tmp_re2,tmp_im1,tmp_im2); 
	      }


	      if (tmp_re1 > 127)
		  tmp_re1 = 127;
	      else if (tmp_re1 < -128)
		tmp_re1 = -128;

	      if (tmp_re2 > 127)
		  tmp_re2 = 127;
	      else if (tmp_re2 < -128)
		tmp_re2 = -128;

	      if (tmp_im1 > 127)
		tmp_im1 = 127;
	      else if (tmp_im1 < -128)
		tmp_im1 = -128;

	      if (tmp_im2 > 127)
		tmp_im2 = 127;
	      else if (tmp_im2 < -128)
		tmp_im2 = -128;

	      dd[nn]                   = tmp_re1>>4;
	      dd[nn+qam16_offset]      = tmp_re2>>4;
	      dd[nn+quadrature_offset] = tmp_im1>>4;
	      dd[nn+qam16_offset2]     = tmp_im2>>4;

	      //	      printf("nn %d : (%d,%d,%d,%d)\n",nn,dd[nn],dd[nn+qam16_offset],dd[nn+quadrature_offset],dd[nn+qam16_offset2]);
	      rep = 0;
	      nn++;
	    }  //rep = 1
	  
	    /*	      printf("deinter16qam nn=%d, aa=%d,ii=%d,n=%d,i=%d,j=%d, (%d,%d) : %d Isymb2 %d Isymb2_16qam %d\n",
		     nn,
		     aa,
		     ii,
		     n,
		     i,
		     j,
		     dd[nn],
		     dd[nn+quadrature_offset],
		     (ii+aa)<<1,
		     Isymb2-(char*)PHY_vars->sach_data[sach_index].rx_sig_f4,
		     Isymb2_16qam-(char*)PHY_vars->sach_data[sach_index].rx_sig_f4);		      
	    */

	  }
	}	  	  
	aa++;
	if (aa==nb_antennas_tx) {
	  aa=0;
	  ii+=NUMBER_OF_CARRIERS_PER_GROUP;
	  i++;
	  if (i==number_of_active_groups){
	    i=0;
	    j+=nb_antennas_tx;
	    if (j==NUMBER_OF_CARRIERS_PER_GROUP) {
	      j=0;
	      //	      off+=number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2       += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2_16qam += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	    }	
	    ii=j;
	  }
	} // aa=nb_tx_antennas
      } // nn<qam16_offset      qam16_offset      = quadrature_offset>>1;
      break;

    case 2: // 64 QAM

      qam64_offseta     = quadrature_offset/3;
      qam64_offsetb     = qam64_offseta<<1;
      qam64_offset2a    = qam64_offseta + quadrature_offset;
      qam64_offset2b    = qam64_offsetb + quadrature_offset;
      
      Isymb2_64qama      = Isymb2 + 2*number_of_active_groups * number_of_symbols * NUMBER_OF_CARRIERS_PER_GROUP;
      Isymb2_64qamb      = Isymb2_64qama + 2*number_of_active_groups * number_of_symbols * NUMBER_OF_CARRIERS_PER_GROUP;
      
      //      for (n=0,nn=0;
      //	   n<qam16_offset;
      //	   n++) {
      nn=0;
      while (nn<qam64_offseta) {
	
	if (j>=NUMBER_OF_SACH_PILOTS) {
	  //	  	  printf("nn %d,dd[nn] %d\n",nn,dd[nn]);
	  
	  // skip and null out punctured bitss
	  while (((dd[nn]&0x80) == 0) && (nn<qam64_offseta)) {
	    //	    	    printf("%d punctured, dd[%d]=%d\n",nn,nn,dd[nn]);
	    dd[nn] = 0;
	    dd[nn+quadrature_offset] = 0;
	    dd[nn+qam64_offseta] = 0;
	    dd[nn+qam64_offset2a] = 0;
	    dd[nn+qam64_offsetb] = 0;
	    dd[nn+qam64_offset2b] = 0;
	    nn++;
	  }

	  if (nn<qam64_offseta) { // not past the end yet

	    switch (nn%6) {
	    case 0: // 0 1 2
	      
	      dd[nn]                    = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offseta]      = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offsetb]      = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+quadrature_offset]  = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2a]     = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2b]     = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 1: // 1 0 2
	      
	      dd[nn+qam64_offseta]      = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn]                    = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offsetb]      = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offset2a]     = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+quadrature_offset]  = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2b]     = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 2: // 1 2 0
	      
	      dd[nn+qam64_offseta]      = Isymb2[((ii+aa)<<1)]>>4;    // Real component
              dd[nn+qam64_offsetb]      = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn]                    = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offset2a]     = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2b]     = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+quadrature_offset]  = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 3: // 2 0 1
	      
	      dd[nn+qam64_offsetb]      = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn]                    = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offseta]      = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offset2b]     = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+quadrature_offset]  = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2a]     = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 4: // 2 1 0
	      
	      dd[nn+qam64_offsetb]      = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offseta]      = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn]                    = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offset2b]     = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2a]     = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+quadrature_offset]  = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 5: // 0 2 1
	      
	      dd[nn]                    = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offsetb]      = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offseta]      = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+quadrature_offset]  = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2b]     = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2a]     = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;
	    }
	    /*
	    printf("deinter64qam nn=%d, aa=%d,ii=%d,n=%d,i=%d,j=%d, (%d,%d) : %d Isymb2 %d Isymb2_16qam %d\n",
		   nn,
		   aa,
		   ii,
		   n,
		   i,
		   j,
		   dd[nn],
		   dd[nn+quadrature_offset],
		   (ii+aa)<<1,
		   Isymb2-(char*)PHY_vars->sach_data[sach_index].rx_sig_f4,
		   Isymb2_64qama-(char*)PHY_vars->sach_data[sach_index].rx_sig_f4);		      
	    */
	    nn++;
	  } // nn<qam64_offseta
	} // j>NUM_PILOTS	  	  
	aa++;
	if (aa==nb_antennas_tx) {
	  aa=0;
	  ii+=NUMBER_OF_CARRIERS_PER_GROUP;
	  i++;
	  if (i==number_of_active_groups){
	    i=0;
	    j+=nb_antennas_tx;
	    if (j==NUMBER_OF_CARRIERS_PER_GROUP) {
	      j=0;
	      //	      off+=number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2        += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2_64qama += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2_64qamb += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	    }	
	    ii=j;
	  }
	} // aa=nb_tx_antennas
      } // nn<qam16_offset
      break;
    default: // Unknown
      msg("[PHY][CODING][SACH] TTI %d: sach.c: phy_decode_sach, unknown coding format, exiting ...\n",mac_xface->frame);
      break;
    } // coding format
  




    // Viterbi Decoding
    
    Zero_Buffer(&PHY_vars->sach_data[sach_index].demod_pdu[0],
		(num_tb*(tb_size_bytes+4))+8);  // +8 to guarantee a multiple of 8 bytes

    for (n=0;n<num_tb;n++) {

#ifdef USER_MODE  
#ifdef DEBUG_PHY 
      if (first_sach_flag == 1) {
	sprintf(fname,"sach_decode_input%d.m",n);
	sprintf(vname,"sach_decode_in%d",n);
      }
      else {
	sprintf(fname,"sach_decode_input%d2.m",n);
	sprintf(vname,"sach_decode_in%d2",n);
      }
      write_output(fname,vname,
		   &PHY_vars->sach_data[sach_index].demod_data[n * ((tb_size_bytes+4)<<4)],
		   (tb_size_bytes+4)<<4,1,4);
#endif // DEBUG_PHY
#endif // USER_MODE  
      
      sach_pdu  = &PHY_vars->sach_data[sach_index].demod_pdu[n*(tb_size_bytes+4)];    
      Sach_payload2 = &Sach_payload[n*tb_size_bytes];
      
            
      phy_viterbi_dot11_sse2(&PHY_vars->sach_data[sach_index].demod_data[n * ((tb_size_bytes+4)<<4)],
			     sach_pdu,
			     (tb_size_bytes+4)<<3);
      
     

      
      oldcrc= *((unsigned int *)(&sach_pdu[tb_size_bytes]));
      oldcrc&=0x00ffffff;
      
      crc = crc24(sach_pdu,
		  tb_size_bytes<<3)>>8;
      
      
      
      
    
#ifdef DEBUG_PHY    
      if (sach_index == 0) {
	msg("Received CRC : %x\n",oldcrc);
	msg("Computed CRC : %x\n",crc);
      }
#endif // DEBUG_PHY
      
      // descramble data
      
      for (i=0;
	   i<tb_size_bytes;
	   i++) {
	sach_pdu[i] = sach_pdu[i] ^ scrambling_sequence[i];
	Sach_payload2[i] = sach_pdu[i];
#ifdef DEBUG_PHY
	if (sach_index >0)
	  msg("TB %d byte %d: %x \n",n,i,sach_pdu[i]);
#endif //DEBUG_PHY
      }
    crc_status[n] = (crc==oldcrc) ? 0 : -SACH_ERROR;

    } // loop over TBs

    _mm_empty();
    
  } //
  else  // SACCH was decoded in error
    crc_status[0] = -SACCH_ERROR;
}

// downlink SACH, same as CHBCH
int phy_sach_phase_comp(struct complex16 *Rchsch, 
			struct complex16 *Rsymb, 
			int chbch_ind, 
			int nb_antennas_tx, 
			struct complex16 *perror_out, 
			unsigned char do_rotate) {
  __m64 *Rchsch64;
  register __m64 mm0,mm1;
  struct complex16 perror;
  int i2,ind,ind64,aatx,norm=0;
  __m64 temp64;
  short *Rsymb64_ptr=NULL,*perror64_ptr=NULL;

  if (NUMBER_OF_CHBCH_PILOTS) {
    perror.r = 0;
    perror.i = 0;

    Rchsch64 = (__m64 *)Rchsch;

    // inner product of received CHSCH in pilot positions and received symbol
    mm1 = _m_pxor(mm1,mm1);

    ind64 = FIRST_CARRIER_OFFSET+(chbch_ind-1)*NUMBER_OF_CHBCH_PILOTS/2*NUMBER_OF_CARRIERS_PER_GROUP;
    ind = ind64<<1;

    for (i2=0;i2<NUMBER_OF_CHBCH_PILOTS;i2+=nb_antennas_tx) {

      if (ind != 0) // skip DC carrier
	for (aatx=0;aatx<nb_antennas_tx;aatx++) {
	  Rsymb64_ptr = (short*)&Rsymb_conj64;
	  Rsymb64_ptr[0] = Rsymb[ind].r;
	  Rsymb64_ptr[1] = Rsymb[ind].i;
	  Rsymb64_ptr[2] = -Rsymb[ind].i;
	  Rsymb64_ptr[3] = Rsymb[ind].r;
	     
	  mm0 = _mm_madd_pi16(Rchsch64[ind64],Rsymb_conj64);
	  mm1 = _mm_add_pi32(mm0,mm1);
	    
#ifdef DEBUG_PHY
#ifdef USER_MODE
	  msg("[OPENAIR][PHY][CHBCH PHASE EST]Ant %d pilot %d (%d,%d): RX p (%d,%d), RX s (%d,%d)\n",
	      aatx,i2,ind64,ind,Rchsch[ind].r,Rchsch[ind].i,
	      Rsymb[ind].r,Rsymb[ind].i);
#endif //USER_MODE
#endif //DEBUG_PHY
	  //	  PHY_vars->chbch_data[chbch_ind].perror.r += ( ((Rchsch[ind].r*Rsymb[ind].r)>>PERROR_SHIFT) + ((Rchsch[ind].i*Rsymb[ind].i)>>PERROR_SHIFT) );
	  //	  PHY_vars->chbch_data[chbch_ind].perror.i += ( ((Rchsch[ind].i*Rsymb[ind].r)>>PERROR_SHIFT) - ((Rchsch[ind].r*Rsymb[ind].i)>>PERROR_SHIFT) );
	  // MMX version
	  ind+=2;
	  ind64++;
	}
      else
	ind64+=nb_antennas_tx;

      temp64 = mm1;

	
      ind64+=(NUMBER_OF_CARRIERS_PER_GROUP-nb_antennas_tx);
      if (ind64>=NUMBER_OF_OFDM_CARRIERS)
	ind64-=NUMBER_OF_OFDM_CARRIERS;
      ind=ind64<<1;
    }


    //PHY_vars->chbch_data[chbch_ind].perror.r >>= LOG2_NUMBER_OF_CHBCH_PILOTS;
    //PHY_vars->chbch_data[chbch_ind].perror.i >>= LOG2_NUMBER_OF_CHBCH_PILOTS;

    perror64 = _mm_srai_pi32(mm1,PERROR_SHIFT+LOG2_NUMBER_OF_CHBCH_PILOTS);
    //        printf("perror64.r = %d, perror64.i=%d\n",((int*)&perror64)[0],((int*)&perror64)[1]);
    perror64_ptr = (short*)&perror64;
    perror.r = perror64_ptr[0];  
    perror.i = perror64_ptr[2];
    //    printf("perror.r = %d, perror.i=%d\n",perror.r,perror.i);

    norm = iSqrt((int)perror.r*perror.r + perror.i*perror.i);
    //    printf("norm %d (%d,%d)\n",norm,perror.r,perror.i);
    // bring perror to unit circle with 8 bits of precision
    if (norm>0) {
      perror.r <<= 5;
      perror.i <<= 5;
      perror.r /= norm;
      perror.i /= norm;
    }

    *perror_out = perror;

    // Apply rotation

    if (do_rotate == 1) 
      rotate_cpx_vector((short *)Rsymb,
			(short *)&perror,
			(short *)Rsymb,
			NUMBER_OF_OFDM_CARRIERS,
			5,
			0);

  }
  return(norm);
}

#ifndef SSSE3
#define abs_pi16(x,zero,res,sign)     sign=_mm_cmpgt_pi16(zero,x) ; res=_mm_xor_si64(x,sign);   //negate negatives
#endif

#ifndef USER_MODE
#define NOCYGWIN_STATIC static
#else
#define NOCYGWIN_STATIC 
#endif

NOCYGWIN_STATIC __m64 ONE_OVER_SQRT_8  __attribute__((aligned(16))); 



NOCYGWIN_STATIC __m64 rho_rpi __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  rho_rmi __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0r_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0i_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1r_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1i_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 logmax_num_re0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_num_im0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_den_re0 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m64  logmax_den_im0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_num_re1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_num_im1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_den_re1 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m64  logmax_den_im1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  A __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  B __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  C __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  D __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  E __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  F __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  G __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  H __attribute__ ((aligned(16))); 


void dual_stream_qpsk_ic_sach(short *stream0_in,
			      short *stream1_in,
			      short *stream0_out,
			      short *rho01,
			      int length,
			      unsigned char interf
			      ) {

  __m64 *rho01_64 = (__m64 *)rho01;
  __m64 *stream0_64_in = (__m64 *)stream0_in;
  __m64 *stream1_64_in = (__m64 *)stream1_in;
  __m64 *stream0_64_out = (__m64 *)stream0_out;


  int i;

  ((short*)&ONE_OVER_SQRT_8)[0] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[1] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[2] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[3] = 23170;

  for (i=0;i<length>>1;i+=2) {



    // STREAM 0

    if (interf == 1) {

      xmm0 = rho01_64[i];
      xmm1 = rho01_64[i+1];
      
      // put (rho_r + rho_i)/2sqrt2 in rho_rpi
      // put (rho_r - rho_i)/2sqrt2 in rho_rmi

      xmm0 = _mm_shuffle_pi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
      xmm1 = _mm_shuffle_pi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));


      xmm2 = _mm_unpacklo_pi32(xmm0,xmm1);
      xmm3 = _mm_unpackhi_pi32(xmm0,xmm1);



      rho_rpi = _mm_adds_pi16(xmm2,xmm3);
      rho_rmi = _mm_subs_pi16(xmm2,xmm3);

      
      rho_rpi = _mm_mulhi_pi16(rho_rpi,ONE_OVER_SQRT_8);
      rho_rmi = _mm_mulhi_pi16(rho_rmi,ONE_OVER_SQRT_8);

      xmm0 = stream0_64_in[i];
      xmm1 = stream0_64_in[i+1];
      
      xmm0 = _mm_shuffle_pi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
      xmm1 = _mm_shuffle_pi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
      y0r  = _mm_unpacklo_pi32(xmm0,xmm1);
      y0r_over2  = _mm_srai_pi16(y0r,1);
      y0i  = _mm_unpackhi_pi32(xmm0,xmm1);
      y0i_over2  = _mm_srai_pi16(y0i,1);
      
      xmm0 = stream1_64_in[i];
      xmm1 = stream1_64_in[i+1];
      
      
      xmm0 = _mm_shuffle_pi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
      xmm1 = _mm_shuffle_pi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
      y1r  = _mm_unpacklo_pi32(xmm0,xmm1);
      y1r_over2  = _mm_srai_pi16(y1r,1);
      y1i  = _mm_unpackhi_pi32(xmm0,xmm1);
      y1i_over2  = _mm_srai_pi16(y1i,1);
      
      // Detection for y0r
      
      xmm0 = _mm_xor_si64(xmm0,xmm0);   // ZERO
      
      xmm3 = _mm_subs_pi16(y1r_over2,rho_rpi); 
      abs_pi16(xmm3,xmm0,A,xmm1);       
      xmm2 = _mm_adds_pi16(A,y0i_over2); 
      xmm3 = _mm_subs_pi16(y1i_over2,rho_rmi); 
      abs_pi16(xmm3,xmm0,B,xmm1);       
      logmax_num_re0 = _mm_adds_pi16(B,xmm2); 
      
      xmm3 = _mm_subs_pi16(y1r_over2,rho_rmi); 
      abs_pi16(xmm3,xmm0,C,xmm1);       
      xmm2 = _mm_subs_pi16(C,y0i_over2); 
      xmm3 = _mm_adds_pi16(y1i_over2,rho_rpi); 
      abs_pi16(xmm3,xmm0,D,xmm1);       
      xmm2 = _mm_adds_pi16(xmm2,D); 
      logmax_num_re0 = _mm_max_pi16(logmax_num_re0,xmm2);  
      
      xmm3 = _mm_adds_pi16(y1r_over2,rho_rmi); 
      abs_pi16(xmm3,xmm0,E,xmm1);       
      xmm2 = _mm_adds_pi16(E,y0i_over2); 
      xmm3 = _mm_subs_pi16(y1i_over2,rho_rpi); 
      abs_pi16(xmm3,xmm0,F,xmm1);       
      logmax_den_re0 = _mm_adds_pi16(F,xmm2); 
      
      xmm3 = _mm_adds_pi16(y1r_over2,rho_rpi); 
      abs_pi16(xmm3,xmm0,G,xmm1);       
      xmm2 = _mm_subs_pi16(G,y0i_over2); 
      xmm3 = _mm_adds_pi16(y1i_over2,rho_rmi); 
      abs_pi16(xmm3,xmm0,H,xmm1);       
      xmm2 = _mm_adds_pi16(xmm2,H); 
      
      logmax_den_re0 = _mm_max_pi16(logmax_den_re0,xmm2);  
      
      // Detection for y0i
      
      xmm2 = _mm_adds_pi16(A,y0r_over2); 
      logmax_num_im0 = _mm_adds_pi16(B,xmm2); 
      
      xmm2 = _mm_subs_pi16(E,y0r_over2); 
      xmm2 = _mm_adds_pi16(xmm2,F); 
      
      logmax_num_im0 = _mm_max_pi16(logmax_num_im0,xmm2);
      
      xmm2 = _mm_adds_pi16(C,y0r_over2); 
      logmax_den_im0 = _mm_adds_pi16(D,xmm2); 
      
      xmm2 = _mm_subs_pi16(G,y0r_over2); 
      xmm2 = _mm_adds_pi16(xmm2,H); 
      
      logmax_den_im0 = _mm_max_pi16(logmax_den_im0,xmm2);  

      y0r = _mm_adds_pi16(y0r,logmax_num_re0);
      y0r = _mm_subs_pi16(y0r,logmax_den_re0);
      
      y0i = _mm_adds_pi16(y0i,logmax_num_im0);
      y0i = _mm_subs_pi16(y0i,logmax_den_im0);


      stream0_64_out[i] = _mm_unpacklo_pi16(y0r,y0i);
      if (i<((length>>1) - 1))
	stream0_64_out[i+1] = _mm_unpackhi_pi16(y0r,y0i);
      
    }
    else {   // No interference here so copy input to output

      /*
      xmm0 = stream0_64_in[i];
      xmm1 = stream0_64_in[i+1];
      
      xmm0 = _mm_shuffle_pi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
      xmm1 = _mm_shuffle_pi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
      y0r  = _mm_unpacklo_pi32(xmm0,xmm1);
      y0i  = _mm_unpackhi_pi32(xmm0,xmm1);
      */
      stream0_64_out[i] = stream0_64_in[i];//_mm_unpacklo_pi16(y0r,y0i);
      if (i<((length>>1) - 1))
	stream0_64_out[i+1] = stream0_64_in[i+1];//_mm_unpackhi_pi16(y0r,y0i);
      


    } // interf=0

    
  //  exit(0);
  }

}


NOCYGWIN_STATIC int rot_mf_sig[8192*4] __attribute__ ((aligned(16)));
NOCYGWIN_STATIC int rot_mf_interf[8192*4] __attribute__ ((aligned(16)));
NOCYGWIN_STATIC int rho_tmp[8192*4] __attribute__ ((aligned(16)));
NOCYGWIN_STATIC int rho[8192*4] __attribute__ ((aligned(16)));

void sach_ic(unsigned char sach_index,        //index of sach transport containing raw RX signal (both signald and interference)
	     unsigned char sch_sig_index,     // sch index of signal component
	     unsigned char sch_interf_index,  // sch index of interference component
	     unsigned char sach_sig_index,    // index of sach transport containing processed RX signal for signal component
	     unsigned char sach_interf_index, // index of sach transport containing processed RX signal for interference component
	     unsigned int nb_antennas_rx,
	     unsigned short freq_alloc,
	     unsigned short ifreq_alloc,
	     unsigned int log2_maxh,
	     unsigned char *I0_shift,
	     unsigned int number_of_symbols,
	     unsigned int first_symbol) {
  
  int i,aa,freq_group,off;
  int *channel_matched_filter_f_sig[NB_ANTENNAS_RX],*channel_matched_filter_f_interf[NB_ANTENNAS_RX];
  unsigned int ind;
  unsigned char interf;

  /*  
  printf("sach_ic: sch_sig_index %d, sch_interf_index %d, sach_sig_index %d, sach_interf_index %d, freq_alloc %x, ifreq_alloc %x\n",sch_sig_index,sch_interf_index,sach_sig_index,sach_interf_index,freq_alloc,ifreq_alloc);
  */

  for (aa=0;aa<nb_antennas_rx;aa++) {
    if (mac_xface->is_cluster_head == 0) {
      channel_matched_filter_f_sig[aa]    = PHY_vars->chsch_data[sch_sig_index].channel_matched_filter_f[aa];
      channel_matched_filter_f_interf[aa] = PHY_vars->chsch_data[sch_interf_index].channel_matched_filter_f[aa];
    }
    else {
      channel_matched_filter_f_sig[aa]    = PHY_vars->sch_data[sch_sig_index].channel_matched_filter_f[aa];
      channel_matched_filter_f_interf[aa] = PHY_vars->sch_data[sch_interf_index].channel_matched_filter_f[aa];
    }
  }

  for (i=first_symbol;
       i<first_symbol+number_of_symbols;
       i++ ){
    

    //    printf("Symbol %d\n",i);


    ind = (FIRST_CARRIER_OFFSET<<1);
    freq_group=0;
      
    while (freq_group < NUMBER_OF_FREQUENCY_GROUPS) {



      if ((freq_alloc & (1<<freq_group))>0) { 
	
	if ((ifreq_alloc & (1<<freq_group))>0)
	  interf=1;
	else
	  interf=0;

	off = ind + (i<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS));

	for (aa=0;aa<NB_ANTENNAS_RX;aa++) {

	  // first rotate matched filters
	  /*	  
	  printf("aa %d, i %d -> perror_sig(%d) (%d,%d)\n",aa,i,sch_sig_index,
		 ((short *)&PHY_vars->sach_data[sch_sig_index].perror[aa][i])[0],
		 ((short *)&PHY_vars->sach_data[sch_sig_index].perror[aa][i])[1]);
	  */
	  rotate_cpx_vector2((short *)&channel_matched_filter_f_sig[aa][ind],
			     (short *)&PHY_vars->sach_data[sch_sig_index].perror[aa][i],
			     (short *)&rot_mf_sig[ind],
			     NUMBER_OF_CARRIERS_PER_GROUP,
			     5,
			     1);

	  if (interf==1) {
	    /*	    
	    printf("aa %d, i %d -> perror_interf(%d) (%d,%d)\n",aa,i,sch_interf_index,
		   ((short *)&PHY_vars->sach_data[sch_interf_index].perror[aa][i])[0],
		   ((short *)&PHY_vars->sach_data[sch_interf_index].perror[aa][i])[1]);
	    */
	    rotate_cpx_vector2((short *)&channel_matched_filter_f_interf[aa][ind],
			       (short *)&PHY_vars->sach_data[sch_interf_index].perror[aa][i],
			       (short *)&rot_mf_interf[ind],
			       NUMBER_OF_CARRIERS_PER_GROUP,
			       5,
			       1);
	  }
	  // matched filter stream 0

	  /*
	  printf("symbol %d freq_group %d ind %d,aa %d\n",i,freq_group,ind,aa);
	  */

	  //	  printf("mf_sig\n");
	  mult_cpx_vector_norep2((short *)&PHY_vars->sach_data[sach_index].rx_sig_f[aa][off],
				(short *)&rot_mf_sig[ind],
				(short *)&PHY_vars->sach_data[sach_sig_index].rx_sig_f2[aa][off>>1],
				NUMBER_OF_CARRIERS_PER_GROUP,
				log2_maxh+I0_shift[aa]); 

	  
	  if (aa>0) {
	    //	    printf("sig accum\n");
	    add_vector16_64((short *)&PHY_vars->sach_data[sach_sig_index].rx_sig_f2[0][off>>1],
			    (short *)&PHY_vars->sach_data[sach_sig_index].rx_sig_f2[aa][off>>1],
			    (short *)&PHY_vars->sach_data[sach_sig_index].rx_sig_f2[0][off>>1],
			    NUMBER_OF_CARRIERS_PER_GROUP<<1);
	  }

	  // matched filter stream 1
	  if (interf==1) {
	    //	    printf("mf_interf\n");
	    mult_cpx_vector_norep2((short *)&PHY_vars->sach_data[sach_index].rx_sig_f[aa][off],
				   (short *)&rot_mf_interf[ind],
				   (short *)&PHY_vars->sach_data[sach_interf_index].rx_sig_f2[aa][off>>1],
				   NUMBER_OF_CARRIERS_PER_GROUP,
				  log2_maxh+I0_shift[aa]);   
	  
	    if (aa>0) {
	      //	      printf("interf accum\n");
	      add_vector16_64((short *)&PHY_vars->sach_data[sach_interf_index].rx_sig_f2[0][off>>1],
			      (short *)&PHY_vars->sach_data[sach_interf_index].rx_sig_f2[aa][off>>1],
			      (short *)&PHY_vars->sach_data[sach_interf_index].rx_sig_f2[0][off>>1],
			      NUMBER_OF_CARRIERS_PER_GROUP<<1);
	    }	  
	  
	    if (aa==0) {
	      
	      // compute complex correlation between stream 0 and 1 relative to strength of stream 0
	      //	      printf("rho\n");
	      mult_cpx_vector_norep_conj2((short *)&rot_mf_interf[ind],
					  (short *)&rot_mf_sig[ind],
					  (short *)&rho[ind>>1],
					  NUMBER_OF_CARRIERS_PER_GROUP,
					  log2_maxh+I0_shift[aa]);
	    }
	    else {
	      
	      // compute complex correlation between stream 0 and 1 relative to strength of stream 0
	      //	      printf("rho\n");
	      mult_cpx_vector_norep_conj2((short *)&rot_mf_interf[ind],
					  (short *)&rot_mf_sig[ind],
					  (short *)&rho_tmp[ind>>1],
					  NUMBER_OF_CARRIERS_PER_GROUP,
					  log2_maxh+I0_shift[aa]);
	      //	      printf("rho accum\n");
	      add_vector16_64((short *)&rho[ind>>1],
			      (short *)&rho_tmp[ind>>1],
			      (short *)&rho[ind>>1],
			      NUMBER_OF_CARRIERS_PER_GROUP<<1);
	      
	    }  // aa>0
	  } // interf=1
	}  // loop on RX antennas

      	
	dual_stream_qpsk_ic_sach((short *)&PHY_vars->sach_data[sach_sig_index].rx_sig_f2[0][off>>1],
				 (short *)&PHY_vars->sach_data[sach_interf_index].rx_sig_f2[0][off>>1],
				 (short *)&PHY_vars->sach_data[sach_sig_index].rx_sig_f2[1][off>>1],
				 &rho[ind>>1],
				 NUMBER_OF_CARRIERS_PER_GROUP,
				 interf
				 );
      } // if freq_group	
      freq_group++;
      ind+=(NUMBER_OF_CARRIERS_PER_GROUP<<1);
      if (ind>=(NUMBER_OF_OFDM_CARRIERS<<1))
	ind-=(NUMBER_OF_OFDM_CARRIERS<<1);	  
      
    } //   while freq_group
  } //     loop on symbols
}



int phy_decode_sach_2streams_ml(int sacch_flag,
				unsigned int first_sach_flag,
				PHY_RESOURCES *Phy_Resources_ptr,
				unsigned char *Sach_payload,
				unsigned char *Sacch_payload,
				unsigned char nb_antennas_rx,
				unsigned char nb_antennas_tx,
				unsigned char sach_index,
				unsigned char sch_index,
				unsigned char stream_index,
				unsigned char num_tb,
				unsigned short tb_size_bytes,
				unsigned int active_processes,
				int *crc_status) { 
  
  int i,ii,first_symbol,j=0,n,aa,iii,nn,ret,ret2;
  int aa_offset;

  unsigned int oldcrc,crc;
  
  unsigned int avg;
  unsigned int i2;
  
  unsigned char log2_maxh;

  int sacch_valid = 1;

  unsigned char coding_fmt;

  unsigned char *Sach_payload2;

  unsigned char *sach_pdu,*sacch_pdu;
  char *dd;
  unsigned int sach_size_encoded_bits,sach_size_bits,sach_size_bytes,sacch_size_input_bits,sacch_size_encoded_bits;
  
#ifdef USER_MODE
  char fname[40],vname[40];
#endif //USER_MODE
  unsigned char time_alloc,coding_rate;
  unsigned short freq_alloc,ifreq_alloc,number_of_symbols;


  unsigned int number_of_active_groups;
  unsigned int number_of_active_carriers;



  char *Isymb2,*Isymb2_16qam,*Isymb2_64qama,*Isymb2_64qamb;
  __m128i *Isymb;


  unsigned int quadrature_offset,qam16_offset,qam16_offset2,qam64_offseta,qam64_offsetb,qam64_offset,qam64_offset2a,qam64_offset2b;      

  int status;

  unsigned char I0_shift[NB_ANTENNAS_RX];
  char I0_min=0,I0_argmin=0,I0_diff,I0_sch_index=0;
  unsigned char rep=0;
  short tmp_re1,tmp_im1,tmp_re2,tmp_im2;

  // check parameters
  if (!Phy_Resources_ptr) {
    msg("[PHY][DECODE SACH] Undefined PHY_Resources, exiting\n");
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);

  }

  if (num_tb>MAX_NUM_TB) {
    msg("[PHY][DECODE SACH] TTI %d: Illegal number of TB, exiting (freq %x, NTb %d)\n",
	mac_xface->frame,
	Phy_Resources_ptr->Freq_alloc,
	num_tb);
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);
  }

  if (tb_size_bytes>MAX_TB_SIZE_BYTES) {
    msg("[PHY][DECODE SACH] Illegal number of TB, exiting\n");
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);
  }

  if ((sacch_flag == 1) && (Sacch_payload == NULL)) {
    msg("[PHY][DECODE SACH] Sacch_payload undefined, exiting\n");
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);
  }
    
  if (Sach_payload == NULL) {
    msg("[PHY][DECODE SACH] Sach_payload undefined, exiting\n");
#ifndef USER_MODE
    openair_sched_exit("");
#endif //USER_MODE
    return(-SACH_PARAM_INVALID);
    
  }

  time_alloc = Phy_Resources_ptr->Time_alloc;
  freq_alloc = Phy_Resources_ptr->Freq_alloc;
  coding_fmt = Phy_Resources_ptr->Coding_fmt;
  if (mac_xface->is_cluster_head == 1)
    ifreq_alloc = Phy_Resources_ptr->Freq_alloc;
  else
    ifreq_alloc = Phy_Resources_ptr->Ifreq_alloc;

  if (mac_xface->is_cluster_head == 1) {
    first_symbol = (int)(((time_alloc >> 4)<<2) - (NUMBER_OF_UL_CONTROL_SYMBOLS));   // top nibble of time_alloc is first symbol in groups of 4 symbols
  }
  else 
    first_symbol = (int)(((time_alloc >> 4)<<2) - FIRST_DL_SACH_SYMBOL);   // top nibble of time_alloc is first symbol in groups of 4 symbols

  
  if (time_alloc == RACH_TIME_ALLOC) {
    number_of_symbols = NUMBER_OF_RACH_SYMBOLS;
    first_symbol=0;
  }
  else {
    number_of_symbols = (unsigned short)((time_alloc & 0xf)<<2);
    // Do a check of SACH parameters to avoid crashing in case of CHBCH_PDU error
    if ( (first_symbol < 0) || 
	 (number_of_symbols >= 64) ) {
      msg("[openair][PHY][CODING] Frame %d: SACH parameter error : first_symbol = %d, number_of_symbols= %d\n",
	  mac_xface->frame,first_symbol,number_of_symbols);
      if (sacch_flag == 1)
	crc_status[0] = -SACH_ERROR;
      else
	crc_status[0] = -SACCH_ERROR;
      return(0);
    }
  }
  
  
#ifdef DEBUG_PHY
  //  if (((mac_xface->frame) % 100) == 0)
    msg("[openair][PHY][CODING] Frame %d: Decode SACH %d, SCH %d (S+N0=%d  N0=%d): Sacch_flag %d, first_sach_flag %d, first_symbol %d, length %d, time_alloc %x, freq_alloc %x, ifreq_alloc %x, Sach_payload %p\n",
	mac_xface->frame,
	sach_index,
	sch_index,
	PHY_vars->PHY_measurements.rx_power_dB[sch_index][0],
	PHY_vars->PHY_measurements.n0_power_dB[sch_index][0],
	sacch_flag,
	first_sach_flag,
	(int)first_symbol,
	number_of_symbols,
	time_alloc,
	freq_alloc,
	ifreq_alloc,
	Sach_payload);
  
  
#endif // DEBUG_PHY 
  
  
    
    // Compute I0 mismatch between receive antennas
    // I0_min and I0_argmin contain the interference level and index of the antenna with the weakest interference
    
    I0_sch_index = (mac_xface->is_cluster_head == 0) ? 0 : 1;
    
    I0_min = PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][0];
    I0_argmin = 0;
    
      
    for (aa=1;aa<nb_antennas_rx;aa++){
      if (I0_min > PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa]) {
	I0_argmin = aa;
	I0_min =  PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa];
      }    
    }
    
    for (aa=0;aa<nb_antennas_rx;aa++) {
      I0_diff = PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa] - I0_min;
      if (I0_diff > 10)
	I0_diff=10;
      I0_shift[aa] = I0_compensation_table[I0_diff];
      //      printf("aa %d: IO_shift %d (%d dB)\n",aa,I0_shift[aa],PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa]);
    }

    /*    
    //  if ((mac_xface->frame % 100) == 0) {
        msg("[OPENAIR][SACH] TTI %d: ",mac_xface->frame);
    for (aa=0;aa<nb_antennas_rx;aa++)
      msg("I0(%d)=%d dBm, I0_shift(%d)=%d  ",aa,PHY_vars->PHY_measurements.n0_power_dB[I0_sch_index][aa],aa,I0_shift[aa]);
    msg("\n");
    //  }
  
    */


  //  for (aa=0;aa<nb_antennas_rx;aa++) {

  aa =I0_argmin;

  // Compute the average signal level on the antenna with lowest inteference/noise level
  sach_channel_level(sch_index,
		     aa,
		     &number_of_active_groups,
		     &avg,
		     (unsigned short) freq_alloc);

  for (aa=0;aa<nb_antennas_rx;aa++) {
    if (mac_xface->is_cluster_head == 0) {
      //      printf("phase_comp sig ant:%d\n",aa);
      ret = sach_phase_error_comp_ue(sach_index,
				     sch_index,
				     sch_index,
				     aa,
				     number_of_symbols,
				     first_symbol,
				     freq_alloc,
				     nb_antennas_tx,
				     first_sach_flag,
				     0);
      // compute phase offset for interference
      //      printf("phase_comp int ant:%d\n",aa);
      ret2 = sach_phase_error_comp_ue(sach_index,
				      (sch_index==1) ? 2 : 1,
				      (sch_index==1) ? 2 : 1,
				      aa,
				      number_of_symbols,
				      first_symbol,
				      freq_alloc,
				      nb_antennas_tx,
				      first_sach_flag,
				      0);
    }
    else {
      //      printf("phase comp sig\n");
      ret = sach_phase_error_comp_ch(sach_index,
				     sch_index,
				     sch_index,
				     stream_index,
				     aa,
				     number_of_symbols,
				     first_symbol,
				     freq_alloc,
				     nb_antennas_tx,
				     0);
      // compute phase offset for interference

      //      printf("phase comp interf\n");
      ret2 = sach_phase_error_comp_ch(sach_index,
				      (sch_index == 1) ? 2 : 1,
				      (sch_index == 1) ? 2 : 1,
				      (stream_index == 0) ? 1 : 0,
				      aa,
				      number_of_symbols,
				      first_symbol,
				      freq_alloc,
				      nb_antennas_tx,
				      0);
    }
	


  }


  if (!ret) {
    
#ifdef DEBUG_PHY      
    if (sach_index >0 )
      msg("[OPENAIR][PHY][CODING][SACH] Frame: %d Abandoning sach: signal too weak \n",mac_xface->frame); 
#endif //DEBUG_PHY
    
    // Abandon decoding, signal is too weak
    if (sacch_flag == 1) 
      crc_status[0]=-SACH_MISSING;
    else
      crc_status[0]=-SACH_MISSING;
    
    return(0);
  }


  number_of_active_carriers = number_of_active_groups * NUMBER_OF_CARRIERS_PER_GROUP;  
  // find maximum bit position of I/Q components for rescaling
  avg/=(number_of_active_carriers);
  
  
  
  log2_maxh = (log2_approx(avg)/2);// + 12 - 15;//(log2_approx(avg))-8;
  
  
  sacch_size_input_bits = (sacch_flag == 0) ? 0 : SACCH_SIZE_BITS;
  sacch_size_encoded_bits = sacch_size_input_bits<<1;
  
  
  
#ifdef USER_MODE
#ifdef DEBUG_PHY
  if (sach_index == 1)
    msg("[openair][PHY][SACH %d] log2_maxh %d\n",sach_index,log2_maxh);
#endif //DEBUG_PHY
#endif //USER_MODE


  //  printf("Doing sach_ic\n");

  sach_ic(sach_index,
	  sch_index,
	  (sch_index==1) ? 2 : 1,
	  sch_index,
	  (sch_index==1) ? 2 : 1,
	  nb_antennas_rx,
	  freq_alloc,
	  ifreq_alloc,
	  log2_maxh,
	  &I0_shift[0],
	  number_of_symbols,
	  first_symbol);



  //  printf("Doing sach detection\n");

  // QPSK/first stage 16QAM portion of Detection, SACCH+SACH
  iii=0;
  sach_detection_stage0_ic(sach_index,
			   sch_index,
			   number_of_symbols,
			   first_symbol,
			   freq_alloc,
			   number_of_active_carriers,
			   &iii);



  // SAVE Internal Signals for OCTAVE
#ifdef USER_MODE  
#ifdef DEBUG_PHY  
    if (first_sach_flag == 1) {
      sprintf(fname,"sach_comp_output.m");
      sprintf(vname,"sach_comp_out");
    }
    else {
      sprintf(fname,"sach_comp_output2.m");
      sprintf(vname,"sach_comp_out2");
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sch_index].rx_sig_f2[1][0],
		 NUMBER_OF_OFDM_CARRIERS*number_of_symbols,  // length
		 1,   // decimation 1
		 1);  // complex 16-bit R/I
    if (first_sach_flag == 1) {
      sprintf(fname,"sach_demod_output.m");
      sprintf(vname,"sach_demod_out");
    }
    else {
      sprintf(fname,"sach_demod_output2.m");
      sprintf(vname,"sach_demod_out2");
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].rx_sig_f3[0][0],
		 number_of_active_carriers*number_of_symbols,  // length
		 1,   // decimation 1
		 1);  // complex 16-bit R/I
    /*
    if (first_sach_flag == 1) {
      sprintf(fname,"sach_magh_16qam_output%d.m",aa);
      sprintf(vname,"sach_magh_16qam_out%d",aa);
    }
    else {
      sprintf(fname,"sach_magh_16qam_output%d2.m",aa);
      sprintf(vname,"sach_magh_16qam_out%d2",aa);
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].mag_channel_f_16qam[aa][0],
		 number_of_active_carriers*number_of_symbols,  // length
		 1,   // decimation 1
		 1);  // complex 16-bit R/I

    if (first_sach_flag == 1) {
      sprintf(fname,"sach_magh_64qama_output%d.m",aa);
      sprintf(vname,"sach_magh_64qama_out%d",aa);
    }
    else {
      sprintf(fname,"sach_magh_64qama_output%d2.m",aa);
      sprintf(vname,"sach_magh_64qama_out%d2",aa);
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sch_index].mag_channel_f_64qama[aa][0],
		 number_of_active_carriers*number_of_symbols,  // length
		 1,   // decimation 1
		 1);  // complex 16-bit R/I

    if (first_sach_flag == 1) {
      sprintf(fname,"sach_magh_64qamb_output%d.m",aa);
      sprintf(vname,"sach_magh_64qamb_out%d",aa);
    }
    else {
      sprintf(fname,"sach_magh_64qamb_output%d2.m",aa);
      sprintf(vname,"sach_magh_64qamb_out%d2",aa);
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].mag_channel_f_64qamb[aa][0],
		 number_of_active_carriers*number_of_symbols,  // length
		 1,   // decimation 1
		 1);  // complex 16-bit R/I
    */
#endif // DEBUG_PHY
#endif // USER_MODE  
  
 
  

  // SACCH Deinterleaving + Data detection (QPSK)


  //  off = 0;
  j=0;
  i=0;

  Isymb = (__m128i *)&PHY_vars->sach_data[sach_index].rx_sig_f4[0];
  Isymb2 = (char*)PHY_vars->sach_data[sach_index].rx_sig_f4;

  ii=0;
  aa=0;



  //
  // **********SACCH Decoding*****************
  //

  if (sacch_flag==1) {

    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY][CODING][SACCH] sacch_size %d, sacch_size bytes %d, freq_alloc %x, time_alloc %x, number_of_active_groups %d, number_of_symbols %d, NUMBER_OF_CARRIERS_PER_GROUP %d\n",SACCH_SIZE_BITS,SACCH_SIZE_BYTES,
	  freq_alloc,time_alloc,
	  number_of_active_groups,number_of_symbols,NUMBER_OF_CARRIERS_PER_GROUP); 
    */
    //    printf("Isymb2 (before sacch) %p\n",Isymb2);
    sacch_deinter(sach_index,stream_index,sacch_size_encoded_bits,
		  number_of_active_groups,nb_antennas_tx,
		  &i,&j,&ii,&aa,&Isymb2);    
    
    

    // Keep ii, i, j and aa alive for SACH below!!!!
    
    
#ifdef USER_MODE  
#ifdef DEBUG_PHY
    if (first_sach_flag == 1) 
      write_output("sacch_decode_input.m","sacch_decode_in",
		   &PHY_vars->sacch_data[sach_index].demod_data[0],
		   SACCH_SIZE_BITS*2,1,4);
    else
      write_output("sacch_decode_input2.m","sacch_decode_in2",
		   &PHY_vars->sacch_data[sach_index].demod_data[0],
		   SACCH_SIZE_BITS*2,1,4);

#endif // DEBUG_PHY
#endif // USER_MODE  
    

    
    // Viterbi Decoding
    
    
    sacch_pdu  = PHY_vars->sacch_data[sach_index].demod_pdu;

    
    Zero_Buffer(sacch_pdu,
		SACCH_SIZE_BYTES+8);  // +8 to guarantee a multiple of 8 bytes
     
    
    
    phy_viterbi_dot11_sse2(PHY_vars->sacch_data[sach_index].demod_data,
			   sacch_pdu,
			   SACCH_SIZE_BITS);
    
    oldcrc= *((unsigned int *)(&sacch_pdu[SACCH_SIZE_BYTES-4]));
    oldcrc&=0x00ffffff;
    
    crc = crc24(sacch_pdu,
		(SACCH_SIZE_BYTES-4)<<3)>>8;
    
    
    
    
    
#ifdef DEBUG_PHY    
    msg("Received CRC : %x\n",oldcrc);
    msg("Computed CRC : %x\n",crc);
#endif // DEBUG_PHY
    
    // descramble data
    
    for (n=0;
	 n<SACCH_SIZE_BYTES-4;
	 n++) {
      sacch_pdu[n] = sacch_pdu[n] ^ scrambling_sequence[n];
      Sacch_payload[n] = sacch_pdu[n];
#ifdef DEBUG_PHY
            msg("%x ",sacch_pdu[n]);
#endif //DEBUG_PHY
    }
#ifdef DEBUG_PHY
         msg("\n");
#endif //DEBUG_PHY

    // Get SACCH Information
    
    if (crc == oldcrc) {  // SACCH CRC passes get coding format for SACH data detection

      //      coding_fmt = ((UL_SACCH_PDU*)Sacch_payload)->Coding_fmt;
      //      coding_fmt = 0;
    }
    else  // SACCH CRC didn't pass 
      sacch_valid = 0;

    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY][CODING][SACCH] Frame %d: sacch_status %d\n",mac_xface->frame,
	  sacch_valid);
    */


  }  // sacch_flag == 1


  if (sacch_valid) {

    switch (coding_fmt&0x7) {

    case 0: // QPSK Single stream : Rate 1
      coding_rate = 1;
      break;
    case 1: // 16-QAM Single stream : Rate 2
    case 3: // QPSK Dual stream     : Rate 2
      coding_rate = 2;
      break;
    case 2: // 64-QAM Single stream : Rate 3
      coding_rate = 3;
      break;
    case 4: // 16-QAM Dual stream : Rate 4
      coding_rate = 4;
      break;
    case 5: // 64-QAM Dual stream : Rate 6
      coding_rate = 6;
      break;
    default:
      coding_rate = 1;
      break;
      
    }

    // sach size after rate matching
    if (mac_xface->is_cluster_head == 1)
      sach_size_bits = (((number_of_active_groups * NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP)*number_of_symbols)- sacch_size_input_bits)*coding_rate<<1; 
    else
      sach_size_bits = (((number_of_active_groups * NUMBER_OF_SACH_DATA_CARRIERS_PER_GROUP)*number_of_symbols))*coding_rate<<1; 

    sach_size_encoded_bits = (num_tb * (tb_size_bytes+4))<<4; 

    /*
      if (((mac_xface->frame/5) % 200) == 0)
      if (sach_index == 1)
      msg("[OPENAIR][PHY][CODING][SACH] sach_size_encoded_bits %d, sach_size_bits %d, sacch_size_input_bits %d, tb_size bytes %d, freq_alloc %x, time_alloc %x, coding fmt %d, number_of_active_groups %d, number_of_symbols %d, NUMBER_OF_CARRIERS_PER_GROUP %d, NUM Pilots %d\n",
      sach_size_encoded_bits,
      sach_size_bits,
      sacch_size_input_bits,
      tb_size_bytes,
      freq_alloc,
      time_alloc,
      coding_fmt,
      number_of_active_groups,
      number_of_symbols,
      NUMBER_OF_CARRIERS_PER_GROUP,
      NUMBER_OF_SACH_PILOTS); 
    */
    

    // second stage 16QAM detection
    if ((coding_fmt&0x7) == 1) {  // do second stream for 16-QAM


      msg("[PHY][SACH] TTI %d: sach.c Should be here, 16-QAM detection\n");
      mac_xface->macphy_exit("");
      return;

      sach16qam_detection(sach_index,
			  nb_antennas_rx,
			  number_of_active_carriers,
			  number_of_active_carriers*number_of_symbols,
			  &iii);
      
    }
    else if ((coding_fmt&0x7) == 2) {

      msg("[PHY][SACH] TTI %d: sach.c Should be here, 64-QAM detection\n");
      mac_xface->macphy_exit("");
      return;

      sach64qam_detection(sach_index,
			  nb_antennas_rx,
			  number_of_active_carriers,
			  number_of_active_carriers*number_of_symbols,
			  &iii);
    }
    

    // SACH Deinterleaving + Data detection (QPSK)
    // retrieving aa, ii, i, and j from above


    dd = PHY_vars->sach_data[sach_index].demod_data;
    
    quadrature_offset = sach_size_encoded_bits>>1;


    // Clear LLR Buffer
    Zero_Buffer(dd,
		sach_size_encoded_bits+8);

    // Tag non-punctured bits
    status = rate_matching(sach_size_bits,
			   sach_size_encoded_bits,
			   dd,
			   2*coding_rate,
			   0);  // offset to be dynamic later


#ifndef USER_MODE
    if (status == -1) {
      msg("[PHY][SACH] sach.c: Rate matching error during SACH decode, freq %x, numtb %d\n",
	  freq_alloc,num_tb);

      openair_sched_exit("[PHY][SACH] sach.c: Rate matching error during SACH decode");
    }
#endif 

#ifdef USER_MODE  
#ifdef DEBUG_PHY  

    //    printf("rx_sig_f4 = %p\n",PHY_vars->sach_data[sach_index].rx_sig_f4);
    if (first_sach_flag == 1) {
      sprintf(fname,"sach_rescale_output0.m");
      sprintf(vname,"sach_rescale_out0");
    }
    else {
      sprintf(fname,"sach_rescale_output02.m");
      sprintf(vname,"sach_rescale_out02");
    }
    write_output(fname,vname,
		 &PHY_vars->sach_data[sach_index].rx_sig_f4[0],
		 number_of_active_carriers*number_of_symbols*coding_rate*2,  // length
		 1,   // decimation 1
		 4);  // real 8-bit
    
#endif // DEBUG_PHY
#endif // USER_MODE  


    //    printf("Isymb2 (before sach) %d\n",Isymb2 - (char*)PHY_vars->sach_data[sach_index].rx_sig_f4);
    switch (coding_fmt&7) {
      case 0 :   // QPSK
	
	//      for (n=0,nn=0;
	//	   n<quadrature_offset;
	//	   n++) {
	nn=0;
	while (nn<quadrature_offset) {

	  aa_offset = (aa+stream_index)%NB_ANTENNAS_TX;
	  if (j>=NUMBER_OF_SACH_PILOTS) {
	    
	    // skip and null out punctured bitss
	    while ((dd[nn]&0x80) == 0) {
	      
	      dd[nn] = 0;
	      dd[nn+quadrature_offset] = 0;
	      //	      	    printf("nn %d punctured\n",nn);  
	      nn++;
	    }
	    //	    printf("dd[%d] = %x ->",nn,dd[nn]);
	    if (nn<quadrature_offset) {
	      if (rep == 0) {
		if (dd[nn] == (char)0x80) {
		  dd[nn]                    = Isymb2[((ii+aa_offset)<<1)]>>4;    // Real component
		  dd[nn+quadrature_offset]  = Isymb2[((ii+aa_offset)<<1)+1]>>4;  // Imaginary components
		  //		  		  printf("nn %d, ii %d, j %d (%d,%d)\n",nn,ii,j,dd[nn],dd[nn+quadrature_offset]); 
		  nn++;
		}
		else {  // store first repeated bit
		  tmp_re1 = Isymb2[((ii+aa_offset)<<1)];
		  tmp_im1 = Isymb2[((ii+aa_offset)<<1)+1];
		  //		  		  printf("(rep) nn %d, ii %d, j %d (%d,%d)\n",nn,ii,j,tmp_re1,tmp_im1); 
		  rep = 1;
		}
	      }
	      else { // Do MR combining of repeated bit
		tmp_re1 += Isymb2[((ii+aa_offset)<<1)];
		if (tmp_re1 > 127)
		  tmp_re1 = 127;
		else if (tmp_re1 < -128)
		  tmp_re1 = -128;
		tmp_im1 += Isymb2[((ii+aa_offset)<<1)+1];
		if (tmp_im1 > 127)
		  tmp_im1 = 127;
		else if (tmp_im1 < -128)
		  tmp_im1 = -128;

		dd[nn]                    = tmp_re1>>4;    // Real component
		dd[nn+quadrature_offset]  = tmp_im1>>4;  // Imaginary components
		  //	    printf("nn %d, ii %d, j %d (%d,%d)\n",nn,ii,j,dd[nn],dd[nn+quadrature_offset]); 
		nn++;
		rep=0;
	      }
	    }
	  }
	  aa++;
	  if (aa==nb_antennas_tx) {
	    aa=0;
	    ii+=NUMBER_OF_CARRIERS_PER_GROUP;
	    i++;
	    if (i==number_of_active_groups){
	      i=0;
	      j+=nb_antennas_tx;
	      if (j==NUMBER_OF_CARRIERS_PER_GROUP) {
		j=0;
		Isymb2 += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      }	
	      ii=j;
	    }  
	  }
	} // nn<quadrature_offset  
	break;

    case 1: // coding_fmt != 0 // 16-QAM
      qam16_offset      = quadrature_offset>>1;
      qam16_offset2     = qam16_offset + quadrature_offset;
      
      Isymb2_16qam      = Isymb2+ 2*number_of_active_groups * number_of_symbols * NUMBER_OF_CARRIERS_PER_GROUP;
      
      //      for (n=0,nn=0;
      //	   n<qam16_offset;
      //	   n++) {
      nn=0;
      while (nn<qam16_offset) {
	 
	if (j>=NUMBER_OF_SACH_PILOTS) {
	  //	  printf("nn %d\n",nn);
	  
	  // skip and null out punctured bitss
	  while ((dd[nn]&0x80) == 0) {
	    //	    	    printf("%d,%d,%d,%d punctured\n",nn,nn+quadrature_offset,nn+qam16_offset,nn+qam16_offset2);
	    dd[nn] = 0;
	    dd[nn+quadrature_offset] = 0;
	    dd[nn+qam16_offset] = 0;
	    dd[nn+qam16_offset2] = 0;
	    nn++;
	  }

	  if (nn<qam16_offset) { // not past the end yet

	    if (rep == 0) {
	      //	      printf("dd[%d] = %x\n",nn,(unsigned char)dd[nn]);
	      if (dd[nn]==(char)0x80) { // bit is not punctured and not repeated
		if (nn&1) { // bit deinterleaving
		  
		  dd[nn]                    = Isymb2[((ii+aa)<<1)]>>4;    // Real component
		  dd[nn+qam16_offset]       = Isymb2_16qam[((ii+aa)<<1)]>>4;    // Real component
		  dd[nn+quadrature_offset]  = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
		  dd[nn+qam16_offset2]      = Isymb2_16qam[((ii+aa)<<1)+1]>>4;  // Imaginary components

		  //		  printf("nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,dd[nn],dd[nn+qam16_offset],dd[nn+quadrature_offset],dd[nn+qam16_offset2]); 
		}
		else {
		  
		  dd[nn+qam16_offset]       = Isymb2[((ii+aa)<<1)]>>4;    // Real component
		  dd[nn]                    = Isymb2_16qam[((ii+aa)<<1)]>>4;    // Real component
		  dd[nn+qam16_offset2]      = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
		  dd[nn+quadrature_offset]  = Isymb2_16qam[((ii+aa)<<1)+1]>>4;  // Imaginary components

		  //		  printf("nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,dd[nn],dd[nn+qam16_offset],dd[nn+quadrature_offset],dd[nn+qam16_offset2]); 
		}
		nn++;
	      }
	      else {   // bit is not punctured but repeated
		
		if (nn&1) { // bit deinterleaving
		  
		  tmp_re1                   = Isymb2[((ii+aa)<<1)];    // Real component
		  tmp_re2                   = Isymb2_16qam[((ii+aa)<<1)];    // Real component
		  tmp_im1                   = Isymb2[((ii+aa)<<1)+1];  // Imaginary components
		  tmp_im2                   = Isymb2_16qam[((ii+aa)<<1)+1];  // Imaginary components
		  //		  printf("(rep0) nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,tmp_re1,tmp_re2,tmp_im1,tmp_im2); 
		}
		else {
		  
		  tmp_re2                   = Isymb2[((ii+aa)<<1)];    // Real component
		  tmp_re1                   = Isymb2_16qam[((ii+aa)<<1)];    // Real component
		  tmp_im2                   = Isymb2[((ii+aa)<<1)+1];  // Imaginary components
		  tmp_im1                   = Isymb2_16qam[((ii+aa)<<1)+1];  // Imaginary components
		  //		  printf("(rep0) nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,tmp_re1,tmp_re2,tmp_im1,tmp_im2); 
		}


		rep = 1;
		
	      }
	    }
	    else {   //rep = 1

	      if (nn&1) { // bit deinterleaving
		
		tmp_re1                   += Isymb2[((ii+aa)<<1)];    // Real component
		tmp_re2                   += Isymb2_16qam[((ii+aa)<<1)];    // Real component
		tmp_im1                   += Isymb2[((ii+aa)<<1)+1];  // Imaginary components
		tmp_im2                   += Isymb2_16qam[((ii+aa)<<1)+1];  // Imaginary components
		//		printf("(rep 1)nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,tmp_re1,tmp_re2,tmp_im1,tmp_im2); 
	      }
	      else {
		
		tmp_re2                   += Isymb2[((ii+aa)<<1)];    // Real component
		tmp_re1                   += Isymb2_16qam[((ii+aa)<<1)];    // Real component
		tmp_im2                   += Isymb2[((ii+aa)<<1)+1];  // Imaginary components
		tmp_im1                   += Isymb2_16qam[((ii+aa)<<1)+1];  // Imaginary component
		//		printf("(rep 1)nn %d, ii %d, j %d aa %d (%d,%d,%d,%d)\n",nn,ii,j,aa,tmp_re1,tmp_re2,tmp_im1,tmp_im2); 
	      }


	      if (tmp_re1 > 127)
		  tmp_re1 = 127;
	      else if (tmp_re1 < -128)
		tmp_re1 = -128;

	      if (tmp_re2 > 127)
		  tmp_re2 = 127;
	      else if (tmp_re2 < -128)
		tmp_re2 = -128;

	      if (tmp_im1 > 127)
		tmp_im1 = 127;
	      else if (tmp_im1 < -128)
		tmp_im1 = -128;

	      if (tmp_im2 > 127)
		tmp_im2 = 127;
	      else if (tmp_im2 < -128)
		tmp_im2 = -128;

	      dd[nn]                   = tmp_re1>>4;
	      dd[nn+qam16_offset]      = tmp_re2>>4;
	      dd[nn+quadrature_offset] = tmp_im1>>4;
	      dd[nn+qam16_offset2]     = tmp_im2>>4;

	      //	      printf("nn %d : (%d,%d,%d,%d)\n",nn,dd[nn],dd[nn+qam16_offset],dd[nn+quadrature_offset],dd[nn+qam16_offset2]);
	      rep = 0;
	      nn++;
	    }  //rep = 1
	  
	    /*	      printf("deinter16qam nn=%d, aa=%d,ii=%d,n=%d,i=%d,j=%d, (%d,%d) : %d Isymb2 %d Isymb2_16qam %d\n",
		     nn,
		     aa,
		     ii,
		     n,
		     i,
		     j,
		     dd[nn],
		     dd[nn+quadrature_offset],
		     (ii+aa)<<1,
		     Isymb2-(char*)PHY_vars->sach_data[sach_index].rx_sig_f4,
		     Isymb2_16qam-(char*)PHY_vars->sach_data[sach_index].rx_sig_f4);		      
	    */

	  }
	}	  	  
	aa++;
	if (aa==nb_antennas_tx) {
	  aa=0;
	  ii+=NUMBER_OF_CARRIERS_PER_GROUP;
	  i++;
	  if (i==number_of_active_groups){
	    i=0;
	    j+=nb_antennas_tx;
	    if (j==NUMBER_OF_CARRIERS_PER_GROUP) {
	      j=0;
	      //	      off+=number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2       += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2_16qam += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	    }	
	    ii=j;
	  }
	} // aa=nb_tx_antennas
      } // nn<qam16_offset      qam16_offset      = quadrature_offset>>1;
      break;

    case 2: // 64 QAM

      qam64_offseta     = quadrature_offset/3;
      qam64_offsetb     = qam64_offseta<<1;
      qam64_offset2a    = qam64_offseta + quadrature_offset;
      qam64_offset2b    = qam64_offsetb + quadrature_offset;
      
      Isymb2_64qama      = Isymb2 + 2*number_of_active_groups * number_of_symbols * NUMBER_OF_CARRIERS_PER_GROUP;
      Isymb2_64qamb      = Isymb2_64qama + 2*number_of_active_groups * number_of_symbols * NUMBER_OF_CARRIERS_PER_GROUP;
      
      //      for (n=0,nn=0;
      //	   n<qam16_offset;
      //	   n++) {
      nn=0;
      while (nn<qam64_offseta) {
	
	if (j>=NUMBER_OF_SACH_PILOTS) {
	  //	  	  printf("nn %d,dd[nn] %d\n",nn,dd[nn]);
	  
	  // skip and null out punctured bitss
	  while (((dd[nn]&0x80) == 0) && (nn<qam64_offseta)) {
	    //	    	    printf("%d punctured, dd[%d]=%d\n",nn,nn,dd[nn]);
	    dd[nn] = 0;
	    dd[nn+quadrature_offset] = 0;
	    dd[nn+qam64_offseta] = 0;
	    dd[nn+qam64_offset2a] = 0;
	    dd[nn+qam64_offsetb] = 0;
	    dd[nn+qam64_offset2b] = 0;
	    nn++;
	  }

	  if (nn<qam64_offseta) { // not past the end yet

	    switch (nn%6) {
	    case 0: // 0 1 2
	      
	      dd[nn]                    = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offseta]      = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offsetb]      = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+quadrature_offset]  = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2a]     = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2b]     = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 1: // 1 0 2
	      
	      dd[nn+qam64_offseta]      = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn]                    = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offsetb]      = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offset2a]     = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+quadrature_offset]  = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2b]     = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 2: // 1 2 0
	      
	      dd[nn+qam64_offseta]      = Isymb2[((ii+aa)<<1)]>>4;    // Real component
              dd[nn+qam64_offsetb]      = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn]                    = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offset2a]     = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2b]     = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+quadrature_offset]  = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 3: // 2 0 1
	      
	      dd[nn+qam64_offsetb]      = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn]                    = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offseta]      = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offset2b]     = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+quadrature_offset]  = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2a]     = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 4: // 2 1 0
	      
	      dd[nn+qam64_offsetb]      = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offseta]      = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn]                    = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offset2b]     = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2a]     = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+quadrature_offset]  = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;

	    case 5: // 0 2 1
	      
	      dd[nn]                    = Isymb2[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offsetb]      = Isymb2_64qama[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+qam64_offseta]      = Isymb2_64qamb[((ii+aa)<<1)]>>4;    // Real component
	      dd[nn+quadrature_offset]  = Isymb2[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2b]     = Isymb2_64qama[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      dd[nn+qam64_offset2a]     = Isymb2_64qamb[((ii+aa)<<1)+1]>>4;  // Imaginary components
	      break;
	    }
	    /*
	    printf("deinter64qam nn=%d, aa=%d,ii=%d,n=%d,i=%d,j=%d, (%d,%d) : %d Isymb2 %d Isymb2_16qam %d\n",
		   nn,
		   aa,
		   ii,
		   n,
		   i,
		   j,
		   dd[nn],
		   dd[nn+quadrature_offset],
		   (ii+aa)<<1,
		   Isymb2-(char*)PHY_vars->sach_data[sach_index].rx_sig_f4,
		   Isymb2_64qama-(char*)PHY_vars->sach_data[sach_index].rx_sig_f4);		      
	    */
	    nn++;
	  } // nn<qam64_offseta
	} // j>NUM_PILOTS	  	  
	aa++;
	if (aa==nb_antennas_tx) {
	  aa=0;
	  ii+=NUMBER_OF_CARRIERS_PER_GROUP;
	  i++;
	  if (i==number_of_active_groups){
	    i=0;
	    j+=nb_antennas_tx;
	    if (j==NUMBER_OF_CARRIERS_PER_GROUP) {
	      j=0;
	      //	      off+=number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2        += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2_64qama += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	      Isymb2_64qamb += 2*number_of_active_groups*NUMBER_OF_CARRIERS_PER_GROUP;
	    }	
	    ii=j;
	  }
	} // aa=nb_tx_antennas
      } // nn<qam16_offset
      break;
    default: // Unknown
      msg("[PHY][CODING][SACH] TTI %d: sach.c: phy_decode_sach, unknown coding format, exiting ...\n",mac_xface->frame);
      break;
    } // coding format
  




    // Viterbi Decoding
    
    Zero_Buffer(&PHY_vars->sach_data[sach_index].demod_pdu[0],
		(num_tb*(tb_size_bytes+4))+8);  // +8 to guarantee a multiple of 8 bytes

    for (n=0;n<num_tb;n++) {

#ifdef USER_MODE  
#ifdef DEBUG_PHY 
      if (first_sach_flag == 1) {
	sprintf(fname,"sach_decode_input%d.m",n);
	sprintf(vname,"sach_decode_in%d",n);
      }
      else {
	sprintf(fname,"sach_decode_input%d2.m",n);
	sprintf(vname,"sach_decode_in%d2",n);
      }
      write_output(fname,vname,
		   &PHY_vars->sach_data[sach_index].demod_data[n * ((tb_size_bytes+4)<<4)],
		   (tb_size_bytes+4)<<4,1,4);
#endif // DEBUG_PHY
#endif // USER_MODE  
      
      sach_pdu  = &PHY_vars->sach_data[sach_index].demod_pdu[n*(tb_size_bytes+4)];    
      Sach_payload2 = &Sach_payload[n*tb_size_bytes];
      
            
      phy_viterbi_dot11_sse2(&PHY_vars->sach_data[sach_index].demod_data[n * ((tb_size_bytes+4)<<4)],
			     sach_pdu,
			     (tb_size_bytes+4)<<3);
      
     

      
      oldcrc= *((unsigned int *)(&sach_pdu[tb_size_bytes]));
      oldcrc&=0x00ffffff;
      
      
      // descramble data
      
      for (i=0;
	   i<tb_size_bytes;
	   i++) {
	sach_pdu[i] = sach_pdu[i] ^ scrambling_sequence[i];
	Sach_payload2[i] = sach_pdu[i];
	//	  msg("TB %d byte %d: %x \n",n,i,sach_pdu[i]);

      }

      /*
      msg("%x %x %x %x\n",sach_pdu[tb_size_bytes],
	     sach_pdu[1+tb_size_bytes],
	     sach_pdu[2+tb_size_bytes],
	     sach_pdu[3+tb_size_bytes]);
      */

      crc = crc24(sach_pdu,
		  tb_size_bytes<<3)>>8;
      
      
      
      
    
#ifdef DEBUG_PHY    
      //      if (sach_index == 0) {
      msg("Received CRC : %x\n",oldcrc);
      msg("Computed CRC : %x\n",crc);
      //      }
#endif // DEBUG_PHY

    crc_status[n] = (crc==oldcrc) ? 0 : -SACH_ERROR;

    } // loop over TBs


    _mm_empty();
    
  } //
  else  // SACCH was decoded in error
    crc_status[0] = -SACCH_ERROR;
}
  
#else //EXPRESSMIMO_TARGET

static unsigned char log2ng[17] = {0,1,1,2,2,2,2,3,3,3,3,3,3,4,4,4,4};
int sach_phase_error_comp_ch(int sach_index,
			     int sch_index,
			     int perror_index,
			     int stream_index,
			     int aa,
			     int number_of_symbols,
			     int first_symbol,
			     unsigned short freq_alloc,
			     int nb_antennas_tx,
			     unsigned char do_rotate) {
}

static struct complex16 perrorv[32];
static int norm;

int sach_phase_error_comp_ue(unsigned char sach_index,
			     unsigned char sch_index,
			     unsigned char perror_index,
			     int aa,
			     int number_of_symbols,
			     int first_symbol,
			     unsigned short freq_alloc,
			     int nb_antennas_tx,
			     unsigned int first_sach_flag,
			     unsigned char do_rotate) {

}

int phy_sach_phase_comp(struct complex16 *Rchsch, 
			struct complex16 *Rsymb, 
			int chbch_ind, 
			int nb_antennas_tx, 
			struct complex16 *perror_out, 
			unsigned char do_rotate) {
}

void sach_channel_compensation(int sach_index,
			       int sch_index,
			       int nb_antennas_rx,
			       short freq_alloc,
			       int log2_maxh,
			       unsigned char *I0_shift,
			       int number_of_symbols,
			       int first_symbol ) {

  int *channel_matched_filter_f,*channel_f,*mag_channel_f,aa,s,ind,freq_group;
  
  // Channel Compensation

  for (aa=0;aa<nb_antennas_rx;aa++){

    if (mac_xface->is_cluster_head == 0) {
      channel_matched_filter_f = PHY_vars->chsch_data[sch_index].channel_matched_filter_f[aa];
      channel_f                = PHY_vars->chsch_data[sch_index].channel_f[aa];
      mag_channel_f            = PHY_vars->chsch_data[sch_index].mag_channel_f[aa];
    }
    else {
      channel_matched_filter_f = PHY_vars->sch_data[sch_index].channel_matched_filter_f[aa];
      channel_f                = PHY_vars->sch_data[sch_index].channel_f[aa];
      mag_channel_f            = PHY_vars->sch_data[sch_index].mag_channel_f[aa];
    }

    // compute magnitude squared of channel

    ind = (FIRST_CARRIER_OFFSET<<1);
    freq_group=0;

    while (freq_group < NUMBER_OF_FREQUENCY_GROUPS) {
      
      if ((freq_alloc & (1<<freq_group))>0) { 

	mult_cpx_vector2((short *)&channel_matched_filter_f[ind],
			 (short *)&channel_f[ind],
			 (short *)&mag_channel_f[ind],
			 NUMBER_OF_CARRIERS_PER_GROUP,
			 log2_maxh+I0_shift[aa]);
      }
      freq_group++;
      ind+=(NUMBER_OF_CARRIERS_PER_GROUP<<1);
      if (ind>=(NUMBER_OF_OFDM_CARRIERS<<1))
	ind-=(NUMBER_OF_OFDM_CARRIERS<<1);	  
      
    }    
#ifdef USER_MODE
#ifdef DEBUG_PHY


#endif //DEBUG_PHY
#endif //USER_MODE

    for (s=0;
	 s<number_of_symbols;
	 s++ ){

      
#ifdef USER_MODE
#ifdef DEBUG_PHY
      msg("[openair][PHY][SACH %d] Compensating symbol %d (%d) shift %d (%d,%d)\n",sach_index,s,s+first_symbol,log2_maxh+I0_shift[aa],log2_maxh,I0_shift[aa]);
#endif //DEBUG_PHY
#endif //USER_MODE
      

      ind = (FIRST_CARRIER_OFFSET<<1);
      freq_group=0;
      
      while (freq_group < NUMBER_OF_FREQUENCY_GROUPS) {
	
	if ((freq_alloc & (1<<freq_group))>0) { 
	  
	  mult_cpx_vector2((short *)&PHY_vars->sach_data[sach_index].rx_sig_f[aa][ind+((first_symbol+s)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS))],
			   (short*)&channel_matched_filter_f[ind],
			   (short *)&PHY_vars->sach_data[sach_index].rx_sig_f2[aa][ind+((first_symbol+s)<<(1+LOG2_NUMBER_OF_OFDM_CARRIERS))],
			   NUMBER_OF_CARRIERS_PER_GROUP,
			   log2_maxh+I0_shift[aa]); 
	}
	freq_group++;
	ind+=(NUMBER_OF_CARRIERS_PER_GROUP<<1);
	if (ind>=(NUMBER_OF_OFDM_CARRIERS<<1))
	  ind-=(NUMBER_OF_OFDM_CARRIERS<<1);	  

      }

    }// symbols
  }// antennas
}

void sach_detection_stage0(int sach_index,
			   int sch_index,
			   int nb_antennas_rx,
			   int number_of_symbols,
			   int first_symbol,
			   unsigned short freq_alloc,
			   int number_of_active_carriers,
			   int *iii) {
}

void sacch_deinter(int sach_index,
		   int stream_index,
		   int sacch_size_encoded_bits,
		   int number_of_active_groups,
		   int nb_antennas_tx,
		   int *i,
		   int *j,
		   int *ii,
		   int *aa,
		   char **Isymb2) {

}

void sach16qam_detection(unsigned int sach_index,
			 unsigned int nb_antennas_rx,
			 unsigned int number_of_active_carriers_per_symbol,
			 unsigned int total_number_of_carriers,
			 unsigned int *iii) {

}


int phy_decode_sach(int sacch_flag,
		    unsigned int first_sach_flag,
		    PHY_RESOURCES *Phy_Resources_ptr,
		    unsigned char *Sach_payload,
		    unsigned char *Sacch_payload,
		    unsigned char nb_antennas_rx,
		    unsigned char nb_antennas_tx,
		    unsigned char sach_index,
		    unsigned char sch_index,
		    unsigned char stream_index,
		    unsigned char num_tb,
		    unsigned short tb_size_bytes,
		    unsigned int active_processes,
		    int *crc_status) { 
}

#endif //EXPRESSMIMO_TARGET


