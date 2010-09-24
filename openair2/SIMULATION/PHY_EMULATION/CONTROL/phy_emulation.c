/*________________________phy_emulation.c________________________

  Authors : Hicham Anouar
  Company : EURECOM
  Emails  : anouar@eurecom.fr
  ________________________________________________________________*/

#include "defs.h"
#include "SIMULATION/PHY_EMULATION/ABSTRACTION/defs.h"
#include "SIMULATION/PHY_EMULATION/ABSTRACTION/misc_proto.h"
#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/extern.h"
#include "SIMULATION/PHY_EMULATION/TRANSPORT/defs.h"
#include "SIMULATION/PHY_EMULATION/TRANSPORT/extern.h"
#include "PHY_INTERFACE/extern.h"
#include "PHY_INTERFACE/defs.h"
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "SIMULATION/simulation_defs.h"


//#define DEBUG_CONTROL 1
//#define DEBUG_LOCAL_RX 1
//#define DEBUG_REMOTE_RX 1
//#define DEBUG_NO_ACK_REQ 1
//#define DEBUG_EMUL_TX 1
extern unsigned int Master_list_rx;

/******************************************************************************************************/ 
char is_node_local_neighbor(unsigned short Node_id){
  /******************************************************************************************************/ 
  int i;
  for(i=0;i<NB_INST;i++)
    if(NODE_ID[i]==Node_id) return 1;
  return 0; 
}

/******************************************************************************************************/ 
void emul_check_out_in_traffic(void){
  /******************************************************************************************************/ 
  /*
    Nb_out_src[0]=0; Nb_out_src[1]=0;//0: CH , 1: UE


    //  Nb_in_src[0]=0;Nb_in_src[1]=0;
    unsigned int i,j,Mark;
  
    for(i=0;i<NB_INST;i++){
    Mark=0;
    for(j=0;j<NB_UE[i];j++)
    if(!is_node_local_neighbor(UE_LIST[i][j])){
    //	Nb_in_src[1]++;
    Mark=1;
    }
    for(j=0;j<NB_CH[i];j++)
    if(!is_node_local_neighbor(CH_LIST[i][j])){
    //	Nb_in_src[0]++;
    Mark=1;
    }
    if(Mark){
    if(NODE_ID[i]<NB_CH_MAX){
    Out_list[0][Nb_out_src[0]++]=i;
    printk("New Out[0] %d\n",i);
    }
    else{ 
    Out_list[1][Nb_out_src[1]++]=i;
    printk("New Out[1] %d\n",i);
    }
    }
    }
    printk("EMULATION:INIT: Check_local_nodes: Nb_out %d,%d\n ",Nb_out_src[0],Nb_out_src[1]);
    //    if(Nb_out_src[0] || Nb_out_src[1] ) exit(0);
    */
}

/******************************************************************************************************/ 
void reset_rssi_meas(void){
  /******************************************************************************************************/ 
  int i,j,k;
  for(i=0;i<NB_INST;i++) 
    for(j=0;j<NUMBER_OF_FREQUENCY_GROUPS;j++) 
      for(k=0;k<NB_TIME_ALLOC;k++)
	Rssi_meas[i][j][k]=0;
  
}

/******************************************************************************************************/ 
void emulation_tx_rx(void){
  /******************************************************************************************************/ 
#ifndef USER_MODE
  char tt=1;
#endif //USER_MODE


  reset_rssi_meas();//for each node, reset the data structure "Rssi_meas" that compute the strength of the recieved signal (combination of signals from all TX nodes) on each frequency groupe and frame phase (CHBCH, UL, DL).   

  reset_rssi_sinr(); //generate channels' realization//replace with a loop of propsim()

  emul_rx_local_measurement();//compute local TXs contribution to "Rssi_meas" 
  
  emul_rx_local_chbch_data();//decoding local CHBCH PDUs
    
  if(!Is_primary_master){////more than one machine in the emulation scenario, this is not the primary master machine
   
    bypass_tx_data(CH_BYPASS_CONTROL);//ALL emulation masters (EM), except the primary master(PM, (machine with Id 0 in the topology script)                                       //send a ctrl msg to the PM to synchronize
                                      //note that there is one EM per machine. Pm & EMs are logical entities, independent from nodes in the emula                                        tion scenario,  used to control the exchange of MAC PDUs on the emulation medium 

    Emulation_status=WAIT_PM_CT;// then wait control msg from the PM
    Master_list_rx=Master_list-1; // e.q: waiting just for the PM
#ifdef DEBUG_CONTROL
    msg("TX CONTROL SIGNAL TO PRIMARY MASTER\n ");
    msg("[EMULATION_CONTROL] Status %d (%d)\n",Emulation_status,WAIT_PM_CT) ; 
#endif

#ifndef USER_MODE
    rtf_put(fifo_mac_bypass,&tt,1);  
#endif //USER_MODE      
    bypass_rx_data();
#ifdef DEBUG_CONTROL
    msg("RX CONTROL SIGNAL FROM PRIMARY MASTER\n ");
#endif //DEBUG_CONTROL
  }

  else if(Master_list){ //more than one machine in the emulation scenario, PM wait for the CTRL msg from other EM   
    Emulation_status=WAIT_EM_CT;
#ifndef USER_MODE
    rtf_put(fifo_mac_bypass,&tt,1);  
#endif //USER_MODE      
    bypass_rx_data();
#ifdef DEBUG_CONTROL
    msg("[EMULATION_CONTROL] Status %d (%d)\n",Emulation_status,WAIT_EM_CT);  
    msg("PM: RX CONTROL SIGNAL FROM SECONDARY MASTERS\n ");
#endif //DEBUG_CONTROL   
    bypass_tx_data(CH_BYPASS_CONTROL);//when the PM receives ctrl msg from all EM, he sends also a CTRL msg to them
#ifdef DEBUG_CONTROL
    msg("PM: TX CONTROL SIGNAL TO SECONDARY MASTERS\n ");
#endif //DEBUG_CONTROL   

  } 
    
  //ALL machines are synchronized, so proceed with data exchange

  if(Master_list){//more than one machine in the emulation scenario

    bypass_tx_data(CHBCH_DATA);//sending CHBCH PDUs  
#ifndef USER_MODE
    rtf_put(fifo_mac_bypass,&tt,1);  
#endif //USER_MODE      

    Emulation_status=WAIT_CHBCH_DATA;//waiting for CHBCH DATA
#ifdef DEBUG_CONTROL
    msg("[EMULATION_CONTROL] Status %d (%d)\n",Emulation_status,WAIT_CHBCH_DATA);  
#endif //DEBUG_CONTROL   
    bypass_rx_data();// receiving raw CHBCH PDUs coming from other machines

    emul_rx_data();  // decoding CHBCH PDUs coming from other machines
    
    bypass_tx_data(UL_DL_DATA);//Transmit DL and UL MAC PDUs

#ifndef USER_MODE
    rtf_put(fifo_mac_bypass,&tt,1);  
#endif //USER_MODE          
    Emulation_status=WAIT_UL_DL_DATA; //After decoding the DL_MAP and UL_MAP, wait for  DL and UL MAC PDUs coming from other machines
#ifdef DEBUG_CONTROL
    msg("[EMULATION_CONTROL] Status %d (%d)\n",Emulation_status,WAIT_UL_DL_DATA);  
#endif //DEBUG_CONTROL   
    bypass_rx_data();//receiving raw DL & UL MAC PDUs, updating RSSI_MEAS structure (equivalent to emul_rx_local_measurement() for local PDUs) 

  }
#ifdef DEBUG_CONTROL
  msg("Emulation Local Procesing\n");
#endif  

  
  emul_rx_local_ul_dl_data();//decoding local DL/UL PDUs

  emul_rx_data();//decoding remote DL/UL PDUs

  clear_non_ack_req();//CHBCH, RACH, and missing sach (UL MAP not decoded correctly or node loosing synchro)

#ifdef DEBUG_CONTROL
  msg("FRAME %d: EMULATION TX/RX Done \n", mac_xface->frame);
#endif  
}

/******************************************************************************************************/ 
void emul_rx_local_measurement(void){
  /******************************************************************************************************/ 
  unsigned short i,j,k,Mod_id,Freq,Time;
  //  char F_k0=0;
  for(Mod_id=0;Mod_id<NB_INST;Mod_id++)
    for(i=0;i<NB_REQ_MAX;i++){
      if(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active ==1
	 && Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Direction == TX){
	for(j=0;j<NB_INST;j++)
	  if(j!=Mod_id){  
	  
	    
	    //UPDATE Interference Io on Phy_resources used by PDU(Mod_id)
	    

	    Freq=Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc;
	    Time=(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc);
	    for(k=0;k<NUMBER_OF_FREQUENCY_GROUPS;k++)
	      if((Freq >> k) & ONE){
		//	if(F_k0==0 && k >0) F_k0=k;
		//			msg("update [%d][%d][%d], RSSI=%d, SINR=%d\n",Emul_idx[j],Emul_idx[Mod_id],k
		//	    ,Rssi[Emul_idx[j]][Emul_idx[Mod_id]][k],Sinr[Emul_idx[j]][k]);
		//		Sinr[Emul_idx[j]][k][Time]=SAT_ADD_FIX(Sinr[Emul_idx[j]][k][Time],Rssi[Emul_idx[j]][Emul_idx[Mod_id]][k]);
		
		Rssi_meas[j][k][Time]=SAT_ADD_FIX(Rssi_meas[j][k][Time],Rssi[Emul_idx[j]][Emul_idx[Mod_id]][k]);////
		/*    msg("[NODE %d][UPDATE LOCAL MEASUREMENT] Node %d TX @ Time_alloc %x, FREQ_alloc %x, Rssi_meas %d, RSSI %d\n",
		      NODE_ID[j],NODE_ID[Mod_id],
		      Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
		      Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc,
		      Rssi_meas[j][k][Time],Rssi[Emul_idx[j]][Emul_idx[Mod_id]][k]);
		*/		
	      }
	 
	    
	    
	    
	    //    Io[j][Antenna_alloc][Freq_alloc]
	    //        =  f(Rssi(Mod_id,j),Phy_resources)
	    
	    //Rssi_inst[Emul_idx[j]][Emul_idx[Mod_id]][Freq_group]=
	    //RSSI[Emul_idx[j]][Emul_idx[Mod_id]]+H_f[Emul_idx[j]][Emul_idx[Mod_id]][Freq_group];
	  }
      }
    }
}

/******************************************************************************************************/ 
void clear_non_ack_req(void){
  /******************************************************************************************************/ 
  unsigned char i,j;
  for(i=0;i<NB_INST;i++){
    for(j=0;j<NB_REQ_MAX;j++){
      if(Macphy_req_table[i].Macphy_req_table_entry[j].Active==1){
	//msg("cleaning entry %d\n",j);
#ifdef DEBUG_NO_ACK_REQ
	msg("INST %d requestiong pdu of TYPE %d on lchan_%d, RX_REQ NOT FILLED!!!\n",i,
	    Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Pdu_type,
	    Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Lchan_id.Index);
#endif //DEBUG_NO_ACK_REQ
	if(((Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Direction == RX))&&(( Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Pdu_type==RACH) || (Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Pdu_type==CHBCH))){
	Macphy_req_table[i].Macphy_req_table_entry[j].Active=0;
	Macphy_req_table[i].Macphy_req_cnt = (Macphy_req_table[i].Macphy_req_cnt - 1)%NB_REQ_MAX;      

	}


	else if(((Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Direction == RX))&&
		( Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Pdu_type==CHBCH_SCH)) {
	  Macphy_req_table[i].Macphy_req_table_entry[j].Active=0;
	  Macphy_req_table[i].Macphy_req_cnt = (Macphy_req_table[i].Macphy_req_cnt - 1)%NB_REQ_MAX;      
	    if((Phy_sync_status[i][Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.CH_index]==SYNC_WAIT)
	       &&( ++Phy_sync_cnt[i][Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.CH_index]
		   > ( PHY_CHBCH_SCH_WAIT_MAX + 50*NODE_ID[i]))){
#ifdef DEBUG_NO_ACK_REQ
	      msg("[PHY_EMULATION]:NODE %d:  NO_SYNCH: init as CH\n",NODE_ID[i]);
#endif //DEBUG_NO_ACK_REQ
	      mac_xface->mrbch_phy_sync_failure(i,NODE_ID[i]%2);
	      Phy_sync_status[i][0]=SYNC_NOK;
	      Phy_sync_status[i][1]=SYNC_NOK;
	    }
	}
	else if(Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Direction == RX){
	  Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Dir.Req_rx.crc_status[0]=-SACH_MISSING;  //
	  mac_xface->macphy_data_ind(i,
				     &Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Dir.Req_rx,
				     Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Pdu_type,
				     Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Lchan_id.Index);  
	  Macphy_req_table[i].Macphy_req_table_entry[j].Active=0;
	  Macphy_req_table[i].Macphy_req_cnt = (Macphy_req_table[i].Macphy_req_cnt - 1)%NB_REQ_MAX;
	  // mac_xface->macphy_exit(""); 
	}
	else if((Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Direction == TX)){
	  Macphy_req_table[i].Macphy_req_table_entry[j].Active=0;
	  Macphy_req_table[i].Macphy_req_cnt = (Macphy_req_table[i].Macphy_req_cnt - 1)%NB_REQ_MAX;
	}
	else {
	  msg("FATAL:------->[FRAME %d]INST %d Requesting pdu of TYPE %d on lchan_%d, on TX, REQ NOT FILLED!!!\n",
	      mac_xface->frame,
	      i,Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Pdu_type,
	      Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Lchan_id.Index );
	  mac_xface->macphy_exit("");
	  
	}
      }
    }
  }
}


/******************************************************************************************************/ 
//Top-level abstraction entry for a UE receiver with Mod_id
void phy_abstraction_ue(unsigned short Src_id, 
			unsigned char Mod_id , 
			MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry){
  /******************************************************************************************************/ 
  int i;
  unsigned short Freq,F_k0,Time,k;
  int N0,Rssi_inst,I0_inst,Sinr_inst,Rssi_sub_band,Intf_sub_band;

  unsigned int U,pe_thres,pe_thres_sacch;

  double signal_strength[NB_SUBCARRIERS_MAX];
  double interference_strength[NB_SUBCARRIERS_MAX];

  Freq=Macphy_data_req_entry->Macphy_data_req.Phy_resources->Freq_alloc;
  Time=Macphy_data_req_entry->Macphy_data_req.Phy_resources->Time_alloc;
  F_k0=0;
  Rssi_inst=0,Rssi_sub_band=0;
  I0_inst=0,Intf_sub_band=0;
  N0=N0_linear;

  // loop through frequency groups and get channel coefficients from propsim

  for(k=0;k<NUMBER_OF_FREQUENCY_GROUPS;k++){

    if((Freq >> k) & ONE){
      if((F_k0==0) && (k >0)) 
	F_k0=k;

      // Get signal components between Emul_idx[Mod_id] and Src_id
      // Need a new function to do this in a generic way
      Rssi_sub_band = Rssi[Emul_idx[Mod_id]][Src_id][k];//dB_fixed(Rssi[Emul_idx[Mod_id]][Src_id][k]);

      // store the signal strengths for carriers in subband
 
      // Get the Interference components between Emul_idx[Mod_id] and all interferers on the same frequency resources
      // How, 
      // For DL, check all other BTS which have allocated band k, by looking at requests from them
      // For UL, check all UL_ALLOC from other BTS which have allocated band k
      // This will not work yet for direct link between UE

      if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == CHBCH) {
	// interference statistics from adjacent CHBCH to Src_id, get aggregate interference stats
	// RK

	Intf_sub_band = -100;

	// store the interference strengths for carriers in subband
      }
      else if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == DL_SACH) {
	// interference statisitcs from adjacent DL-SACH which use the same resources as this one
	// RK

	// store the interference strengths for carriers in subband
	Intf_sub_band = -100; 
      }

      
      // Store Measurements, get them from CHBCH on DL and form UL_SACH on UL
      if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == CHBCH)
	if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas !=NULL)
	  // Store measurements
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Sub_band_sinr[k]=Rssi_sub_band-Intf_sub_band;
      else if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == UL_SACH)
	if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas !=NULL)
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Sub_band_sinr[k]=Rssi_sub_band-Intf_sub_band;

    }
  }


  I0_inst=-100;//dB_fixed(I0_inst);//+dB_fixed(rssi_dB_2_fixed(N0_dB));
  Rssi_inst=Rssi[Emul_idx[Mod_id]][Src_id][0] + NB_SUBBANDS_IN_dB;//xed(Rssi_inst);//+dB_fixed(rssi_dB_2_fixed(N0_dB));

  if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == CHBCH)
    if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas !=NULL){
      //      printf("CHBCH %d rssi %d\n",Macphy_data_req_entry->Macphy_data_req.CH_index,Rssi_inst);
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Wideband_rssi_dBm=Rssi_inst;
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Wideband_sinr_dB=Rssi_inst - I0_inst;
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Wideband_interference_level_dBm=I0_inst;
    }
    else {}
  else if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == UL_SACH)
    if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas !=NULL){
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Wideband_rssi_dBm=Rssi_inst;
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Wideband_interference_level_dBm=I0_inst;
    }

  Sinr_inst=(Rssi_inst-I0_inst);

  if ((mac_xface->frame % 100000) == 0)
    msg("[PHY][EMULATION] TTI %d, Inst %d, Src %d: RSSI = %d dBm, I+N0 = %d dBm, SINR = %d dB\n",
	mac_xface->frame,
	Mod_id,
	Src_id,
	Rssi_inst,
	I0_inst,
	Sinr_inst);

  //	      Sinr[Emul_idx[j]][Emul_idx[Mod_id]][F_k0]=
  
  //SAT_ADD_FIX(Sinr[Emul_idx[j]][Emul_idx[Mod_id]][F_k0],-dB_fixed(ONE<<31));
  //	      Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_rx.num_tb
  // = macphy_get_num_tb(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources);
  
    
  
  //  if(I0_inst > 40)	      exit(0);
  // if(mac_xface->frame%100==0)
  //  msg("[PHY_EMULATION][TTI: %d][NODE %d]RX_DATA from Node %d with SINR %d on Lchan_id %d\n",
  //  mac_xface->frame,
  //  NODE_ID[Mod_id],Src_id,Sinr_inst,Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);
  //calculate CRC_Status vector



  // Error probability abstraction goes here
  // ID
  // Define a function: unsigned int get_transport_block_bler(signal_strength,interference_strength,tb_size_bits,sacch_size_bits,sacch_carrier_alloc)

  
  if(Sinr_inst >= 40) {//RX_SINR_TRESHOLD)
    pe_thres = 0;
    pe_thres_sacch =0;
  }
  else if(Sinr_inst >= 30) {//RX_SINR_TRESHOLD)
    pe_thres = 1000;
    pe_thres_sacch = 200;
  }
  else if (Sinr_inst >= 20) {
    pe_thres = 10000;
    pe_thres_sacch = 2000;
  }
  else if (Sinr_inst >= 10) {
    pe_thres = 100000;
    pe_thres_sacch = 20000;
  }
  else{
    pe_thres = 350000;
    pe_thres_sacch = 350000;
  }

  // Fill in the error stats for MAC
    for (i=0;i<Macphy_data_req_entry->Macphy_data_req.num_tb;i++) {


      if ( (i==0) && (Macphy_data_req_entry->Macphy_data_req.Pdu_type==UL_SACH)) {  // Handle SACCH error condition
	U = taus() % 1000000;
	if (U < pe_thres_sacch)
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=-SACCH_ERROR;  //SACCH_ERROR
	else {
	  U = taus() % 1000000;
	  if (U < pe_thres)
	    Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=-SACH_ERROR;  //SACH_ERROR
	  else
	    Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=0;  //SACH_ERROR
	}
      }
      else{  // i>0 or not UL_SACH
	U = taus() % 1000000;

	if ( (U < pe_thres) || (Sinr_inst <10 )){
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=-SACH_ERROR;  //SACCH_ERROR
	  //if ( Macphy_data_req_entry->Macphy_data_req.Pdu_type==CHBCH)
	    //		msg("[PHY_EMULATION]  TTI %d: CHBCH IN ERROR: Sinr_inst %d, U %d\n",
	    //    mac_xface->frame,Sinr_inst,U); 
	}
	else
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=0;  //
	
      }
    //      msg("_________________________________________[PHY_EMULATION][NODE %d] FRAME%d :RX_DATA from Node %d with SINR %d on Lchan_id %d\n",mac_xface->frame,
    // NODE_ID[Mod_id],Src_id,Sinr_inst,Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);
    
  }
  //fill measurement

}


void phy_abstraction_ch(unsigned short Src_id, 
			unsigned char Mod_id , 
			MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry){

  /******************************************************************************************************/ 
  int i;
  unsigned short Freq,F_k0,Time,k;
  int N0,Rssi_inst,I0_inst,Sinr_inst,Rssi_sub_band,Intf_sub_band;

  unsigned int U,pe_thres,pe_thres_sacch;
  Freq=Macphy_data_req_entry->Macphy_data_req.Phy_resources->Freq_alloc;
  Time=Macphy_data_req_entry->Macphy_data_req.Phy_resources->Time_alloc;
  F_k0=0;
  Rssi_inst=0,Rssi_sub_band=0;
  I0_inst=0,Intf_sub_band=0;
  N0=N0_linear;

  double signal_strength[NB_SUBCARRIERS_MAX];
  double interference_strength[NB_SUBCARRIERS_MAX];

  // loop through frequency groups and get channel coefficients from propsim

  for(k=0;k<NUMBER_OF_FREQUENCY_GROUPS;k++){

    if((Freq >> k) & ONE){
      if((F_k0==0) && (k >0)) 
	F_k0=k;

      // Get signal components between Emul_idx[Mod_id] and Src_id
      // Need a new function to do this in a generic way
      Rssi_sub_band = Rssi[Emul_idx[Mod_id]][Src_id][k];//dB_fixed(Rssi[Emul_idx[Mod_id]][Src_id][k]);

      // Get the Interference components between Emul_idx[Mod_id] and all interferers on the same frequency resources
      // How, 
      // For DL, check all other BTS which have allocated band k, by looking at requests from them
      // For UL, check all UL_ALLOC from other BTS which have allocated band k
      // RK
      Intf_sub_band = -100;//dB_fixed(SAT_ADD_FIX(Rssi_meas[Mod_id][k][Time], -Rssi[Emul_idx[Mod_id]][Src_id][k]));


      
      // Store Measurements, get them from CHBCH on DL and form UL_SACH on UL
      if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == CHBCH)
	if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas !=NULL)
	  // Store measurements
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Sub_band_sinr[k]=Rssi_sub_band-Intf_sub_band;
      else if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == UL_SACH)
	if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas !=NULL)
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Sub_band_sinr[k]=Rssi_sub_band-Intf_sub_band;


      // compute signal and interference strength vectors for carriers in subband k
    }
  }


  I0_inst=-100;//dB_fixed(I0_inst);//+dB_fixed(rssi_dB_2_fixed(N0_dB));

  // Get instantaneous RSSI, this should be filled in by propsim
  Rssi_inst=Rssi[Emul_idx[Mod_id]][Src_id][0] + NB_SUBBANDS_IN_dB;//xed(Rssi_inst);//+dB_fixed(rssi_dB_2_fixed(N0_dB));

  if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == CHBCH)
    if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas !=NULL){
      //      printf("CHBCH %d rssi %d\n",Macphy_data_req_entry->Macphy_data_req.CH_index,Rssi_inst);
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Wideband_rssi_dBm=Rssi_inst;
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Wideband_sinr_dB=Rssi_inst - I0_inst;
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Wideband_interference_level_dBm=I0_inst;
    }
    else {}
  else if (Macphy_data_req_entry->Macphy_data_req.Pdu_type == UL_SACH)
    if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas !=NULL){
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Wideband_rssi_dBm=Rssi_inst;
      Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Wideband_interference_level_dBm=I0_inst;
    }

  Sinr_inst=(Rssi_inst-I0_inst);

  if ((mac_xface->frame % 100000) == 0)
    msg("[PHY][EMULATION] TTI %d, Inst %d, Src %d: RSSI = %d dBm, I+N0 = %d dBm, SINR = %d dB\n",
	mac_xface->frame,
	Mod_id,
	Src_id,
	Rssi_inst,
	I0_inst,
	Sinr_inst);

  // Error probability abstraction goes here
  // ID
  // Define a function: get_transport_block_bler(?)

  if(Sinr_inst >= 40) {//RX_SINR_TRESHOLD)
    pe_thres = 0;
    pe_thres_sacch =0;
  }
  else if(Sinr_inst >= 30) {//RX_SINR_TRESHOLD)
    pe_thres = 1000;
    pe_thres_sacch = 200;
  }
  else if (Sinr_inst >= 20) {
    pe_thres = 10000;
    pe_thres_sacch = 2000;
  }
  else if (Sinr_inst >= 10) {
    pe_thres = 100000;
    pe_thres_sacch = 20000;
  }
  else{
    pe_thres = 350000;
    pe_thres_sacch = 350000;
  }

  // Fill in the error stats for MAC
    for (i=0;i<Macphy_data_req_entry->Macphy_data_req.num_tb;i++) {


      if ( (i==0) && (Macphy_data_req_entry->Macphy_data_req.Pdu_type==UL_SACH)) {  // Handle SACCH error condition
	U = taus() % 1000000;
	if (U < pe_thres_sacch)
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=-SACCH_ERROR;  //SACCH_ERROR
	else {
	  U = taus() % 1000000;
	  if (U < pe_thres)
	    Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=-SACH_ERROR;  //SACH_ERROR
	  else
	    Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=0;  //SACH_ERROR
	}
      }
      else{  // i>0 or not UL_SACH
	U = taus() % 1000000;

	if ( (U < pe_thres) || (Sinr_inst <10 )){
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=-SACH_ERROR;  //SACCH_ERROR
	  //if ( Macphy_data_req_entry->Macphy_data_req.Pdu_type==CHBCH)
	    //		msg("[PHY_EMULATION]  TTI %d: CHBCH IN ERROR: Sinr_inst %d, U %d\n",
	    //    mac_xface->frame,Sinr_inst,U); 
	}
	else
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[i]=0;  //
	
      }
    //      msg("_________________________________________[PHY_EMULATION][NODE %d] FRAME%d :RX_DATA from Node %d with SINR %d on Lchan_id %d\n",mac_xface->frame,
    // NODE_ID[Mod_id],Src_id,Sinr_inst,Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);
    
  }
  //fill measurement


}

/******************************************************************************************************/ 
void emul_rx_local_chbch_data(void){
  /******************************************************************************************************/ 
  unsigned short i,j,Mod_id,kk;
  unsigned char CH_index;
  MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry=NULL;
  MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry_sch=NULL;
  MACPHY_REQ_ENTRY_KEY Search_key;

  // loop over instances in this machine
  for(Mod_id=0;Mod_id<NB_INST;Mod_id++)
    // loop over all requests from MAC
    for(i=0;i<NB_REQ_MAX;i++){
      // if a request from MAC is active transmission of CHBCH, so Mod_id is a CH
      // save the allocation information for lookup (CH_index mainly) for UEs
      if(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active ==1
	 && Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Direction == TX
	 && (Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type == CHBCH)){
	Search_key.Key_type=PHY_RESOURCES_KEY;
	Search_key.Key.Phy_resources.Time_alloc
	  =Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc;
	Search_key.Key.Phy_resources.Freq_alloc
	  =Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc;
	Search_key.CH_index
	  =Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.CH_index;

	for(j=0;j<NB_INST;j++)
	  // node j is not the target CH
	  if( Mod_id!=j){
	    
	    // Get the RX data_req for node j with that CH (Mod_id)
	    Macphy_data_req_entry = find_data_req_entry(j,&Search_key);
	    if (Macphy_data_req_entry) {

#ifdef DEBUG_LOCAL_RX      
	      msg("[NODE %d]RX LOCAL_DATA] from NODE %d @ Time_alloc %x, FREQ_alloc %x\n",
		  NODE_ID[j],NODE_ID[Mod_id],
		  Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
		  Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc);
#endif //DEBUG_LOCAL_RX
	      
	      
	      //COMPUTE SINR/Error patterns on CHBCH PDU from Mod_id to j
	      phy_abstraction_ue(Emul_idx[Mod_id],j,Macphy_data_req_entry);

	      
	      CH_index=(Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index & RAB_OFFSET2) >> RAB_SHIFT2;	      

#ifdef DEBUG_LOCAL_RX
	      msg("__________________[OPENAIR][PHY][EMUL] Inst %d: CHBCH Detected from index %d, %d________________\n",j,CH_index,Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.CH_index);
#endif //DEBUG_LOCAL_RX

	      if((Macphy_data_req_entry->Macphy_data_req.Pdu_type == CHBCH_SCH)){
		if (Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]==0)
		  mac_xface->chbch_phy_sync_success(j,Macphy_data_req_entry->Macphy_data_req.CH_index);
	       
	      }
	      else{
		Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Chbch_pdu
		  = Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.Chbch_pdu;
		
#ifdef DEBUG_LOCAL_RX		
		msg("[OPENAIR][PHY][EMUL Node %d]TTI %d: Detected a CHBCH from CH %d\n",
		    NODE_ID[j],mac_xface->frame,CH_index);  	
#endif //DEBUG_LOCAL_RX
		
		if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]==0){
		  if(Sync_status[j][CH_index]==SYNC_WAIT)
		    if(++Sync_cnt[j][CH_index] > SYNC_OK_TRESHOLD){
		      Sync_status[j][CH_index]=SYNC_OK;
		      Sync_cnt[j][CH_index]=0;
#ifdef DEBUG_LOCAL_RX
		      msg("______________[NODE %d][PHY_EMULATION] Frame %d: PHY_SYNCHRONIZATION TO CH %d[%d]____________\n",
			  NODE_ID[j],mac_xface->frame,CH_index,NODE_ID[Mod_id]);
#endif //DEBUG_LOCAL_RX
		    }
		  
		  if(Sync_status[j][CH_index]==SYNC_OK){
		    mac_xface->macphy_data_ind(j,
					       &Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,
					       CHBCH,
					       Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);  
		    Sync_cnt[j][CH_index]=0;
		    
		  }
		}
		else if(Sync_status[j][CH_index]==SYNC_OK){
		  
		  if(++Sync_cnt[j][CH_index] >= SYNC_NOK_TRESHOLD){
#ifdef DEBUG_LOCAL_RX
		    msg("________________[NODE %d][PHY_EMULATION]Frame %d: PHY_SYNCHRONIZATION TO CH %d[%d] LOST, INDICATE TO RRC_____\n",
			NODE_ID[j],mac_xface->frame,CH_index,NODE_ID[Mod_id]);
#endif DEBUG_LOCAL_RX
		    mac_xface->out_of_sync_ind(j,CH_index);
		    Sync_status[j][CH_index]=SYNC_WAIT;
		    Sync_cnt[j][CH_index]=0;
		  }
		  else
		    mac_xface->macphy_data_ind(j,
					       &Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,
					       CHBCH,
					       Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);  
		}
		else if(Sync_status[j][CH_index]==SYNC_WAIT){
		  Sync_cnt[j][CH_index]=0;
		  //		msg("[PHY EMULATION]TTI %d: CHBCH IN ERROR\n",mac_xface->frame);
		}
		
	      }
	      Macphy_data_req_entry->Active = 0;
	      Macphy_req_table[j].Macphy_req_cnt = (Macphy_req_table[j].Macphy_req_cnt - 1)%NB_REQ_MAX;  
	      
	    }
	  }
	
      }
      
    }
}

//  ?????

/*
void get_local_chbch_allocations() {

}

void get_local_dlsach_allocations() {

}

void get_local_ulsach_allocations() {

}

void get_local_rach_allocations() {

}
*/


/******************************************************************************************************/ 
void emul_rx_local_ul_dl_data(void){
  /******************************************************************************************************/ 
  unsigned short i,j,Mod_id;
  unsigned char CH_index;
  MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry=NULL;
  MACPHY_REQ_ENTRY_KEY Search_key;



  //loop over all instances in the Machine
  for(Mod_id=0;Mod_id<NB_INST;Mod_id++)
    //loop over all requests for the instance
    for(i=0;i<NB_REQ_MAX;i++){
      // if a channel is found which is not CHBCH, handle it
      if((Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active ==1)
	 &&( Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Direction == TX)
	 && (Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type != CHBCH)){

#ifdef DEBUG_CONTROL
	msg("[PHY_EMULATION]Frame %d: EMUL_UL_DL: NODE %d TX on %x, %x \n", mac_xface->frame,
	    NODE_ID[Mod_id],
	    Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
	    Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc);
#endif //DEBUG_CONTROL

	Search_key.Key_type=PHY_RESOURCES_KEY;
	Search_key.Key.Phy_resources.Time_alloc
	 = Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc;
	Search_key.Key.Phy_resources.Freq_alloc
	  =Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc;
	Search_key.CH_index
	  =Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.CH_index;

	// look for the receiver of this packet using the PHY_RESOURCES allocation as a search key
	// Note: need to add antenna index to this for future SDMA support
	for(j=0;j<NB_INST;j++)
	  if( Mod_id!=j){
    
	    Macphy_data_req_entry = find_data_req_entry(j,&Search_key);

	    // if the receiver of this packet is found
	    if (Macphy_data_req_entry) {

#ifdef DEBUG_CONTROL	      
	      msg("[NODE %d]RX LOCAL_DATA] from NODE %d @ Time_alloc %x, FREQ_alloc %x\n",
		  NODE_ID[j],NODE_ID[Mod_id],
		  Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
		  Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc);
#endif //DEBUG_CONTROL	      
	      
	      
	      //COMPUTE SINR and error patterns

	      switch(Macphy_data_req_entry->Macphy_data_req.Pdu_type){
	      case RACH:
#ifdef DEBUG_CONTROL
		msg("__________________[OPENAIR][PHY][EMUL] Inst %d: RACH Detected__________________\n",j);
#endif //DEBUG_CONTROL

		phy_abstraction_ch(Emul_idx[Mod_id],j,Macphy_data_req_entry);

		Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Rach_pdu
		  = &Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.Rach_pdu;
		mac_xface->macphy_data_ind(j,&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,RACH
					   ,Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);
		
		break;
	      case UL_SACH:
#ifdef DEBUG_CONTROL
		msg("__________________[OPENAIR][PHY][EMUL] Inst %d: UL_SACH Detected CRC %d__________________\n",j,Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]);	       
#endif //DEBUG_CONTROL

		phy_abstraction_ch(Emul_idx[Mod_id],j,Macphy_data_req_entry);

		// copy PDU to req buffer
		memcpy(&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.UL_sach_pdu,
		       (UL_SACH_PDU*)&Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.UL_sach_pdu,
		       sizeof(UL_SACH_PDU));

		// send packets up
		mac_xface->macphy_data_ind(j,
					   &Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,
					   UL_SACH,
					   Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);
		break;

	      case DL_SACH:
		phy_abstraction_ue(Emul_idx[Mod_id],j,Macphy_data_req_entry);
#ifdef DEBUG_CONTROL
		msg("__________________[OPENAIR][PHY][EMUL] Inst %d: DL_SACH Detected at LCHAN_ID %d , DL_SACH_RX @ %p, Tx @, %p %p, CRC %d__________________\n",j,Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Lchan_id.Index,
		    &Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.DL_sach_pdu,
		    &Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.DL_sach_pdu,
		    Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.DL_sach_pdu.Sach_payload,
		  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0] );
#endif //DEBUG_CONTROL
		// Copy PDU to RX REQ
		memcpy(&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.DL_sach_pdu,
		       (DL_SACH_PDU*)&Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.DL_sach_pdu,
		       sizeof(DL_SACH_PDU));
		// Send packets up
		mac_xface->macphy_data_ind(j,&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,DL_SACH
					   ,Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);

		break;
	      }
	      Macphy_data_req_entry->Active = 0;
	      Macphy_req_table[j].Macphy_req_cnt = (Macphy_req_table[j].Macphy_req_cnt - 1)%NB_REQ_MAX;  
	    }
	  }
	Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active=0;
	Macphy_req_table[Mod_id].Macphy_req_cnt = (Macphy_req_table[Mod_id].Macphy_req_cnt - 1)%NB_REQ_MAX;	 
      }
       
    }
}


/****************************************************************************************************/
unsigned int emul_tx_handler(u8 Mode,char *Tx_buffer,unsigned int* Nbytes,unsigned int *Nb_flows){
  /****************************************************************************************************/
  unsigned short i,k,Mod_id,W_idx,Tx_size,Pdu_type;
  
  for(k=0;k<NB_INST;k++){//Nb_out_src[Mode];k++){
    Mod_id=k;//Out_list[Mode][k];
    for(i=0;i<NB_REQ_MAX;i++){
      if(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active 
	 && Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Direction == TX &&
	 ((Mode==CHBCH_DATA &&Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type==CHBCH ) || 
	  (Mode!=CHBCH_DATA && Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type!=CHBCH))){

	(*Nb_flows)++;
 	W_idx=*Nbytes;
	(*Nbytes)+=sizeof(unsigned short);

	//Element too identify a flow: Source Emul_id, CH_index, Time_alloc, Freq_alloc //+ (antenna, power)  

	memcpy(&Tx_buffer[*Nbytes],(unsigned short*)&Emul_idx[Mod_id],sizeof(unsigned short)); 
	(*Nbytes)+=sizeof(unsigned short);

	memcpy(&Tx_buffer[*Nbytes],
	       (unsigned short*)&Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc,sizeof(unsigned short)); 
	(*Nbytes)+=sizeof(unsigned short);

	Tx_buffer[(*Nbytes)++]
	  =Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc;


	memcpy(&Tx_buffer[*Nbytes],(unsigned short*)&Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.CH_index,sizeof(unsigned short)); 
	(*Nbytes)+=sizeof(unsigned short);

	

#ifdef DEBUG_EMUL_TX
	msg("frame %d, RADIO_EMULATION_TX: LCHAN_INDEX %d, Time_alloc %x, FREQ_alloc %x, CH_index %d\n",mac_xface->frame,
	    Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Lchan_id.Index,
	    Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
	    Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc,
	    Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.CH_index);
#endif //DEBUG_EMUL_TX

	
	serialize(&Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx,Tx_buffer,Nbytes
		  ,Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type);
	
	Tx_size=(*Nbytes)-(W_idx+2);
	memcpy(&Tx_buffer[W_idx],(unsigned short *)&Tx_size,sizeof(unsigned short));
	
      }
    }
  }
  
  return *Nbytes;
}



/****************************************************************************************************/
unsigned int emul_rx_data(void){
  /****************************************************************************************************/
  unsigned int Mod_id,i;
  unsigned short CH_index;
  MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry=NULL;		
  for(Mod_id=0;Mod_id<NB_INST;Mod_id++)
    for(i=0;i<NB_REQ_MAX;i++)
      if(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active 
	 && Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Direction == RX_READY){

	if((Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type==CHBCH)
	   ||(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type==CHBCH_SCH)
	   ||(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type==DL_SACH))
	  
	  phy_abstraction_ue(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Src_id,
			     Mod_id,&Macphy_req_table[Mod_id].Macphy_req_table_entry[i]);	
	else
	  
	  phy_abstraction_ch(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Src_id,
			     Mod_id,&Macphy_req_table[Mod_id].Macphy_req_table_entry[i]);	

	  //phy_compute_sinr(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Src_id,
	  //	 Mod_id,&Macphy_req_table[Mod_id].Macphy_req_table_entry[i]);	
	//COMPUTE SINR
	//Sinr=f(RSSI(Mod_id,j),Io(Phy_resources))
	//fill measurement
	//calculate CRC_Status vector
	//if(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc>0)
	//	   msg("[NODE %d][EMUL_RX_DATA] Time_alloc %x, FREQ_alloc %x\n",NODE_ID[Mod_id],
	//Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
	//Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc);

	if((Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type == CHBCH_SCH)){ 
	  if (Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_rx.crc_status[0]==0)
	    mac_xface->chbch_phy_sync_success(Mod_id,Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.CH_index);
	}

	else if(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type==CHBCH){
	  Macphy_data_req_entry=&Macphy_req_table[Mod_id].Macphy_req_table_entry[i];
	  CH_index=Macphy_data_req_entry->Macphy_data_req.CH_index;


	  if(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]==0){
	    if(Sync_status[Mod_id][CH_index]==SYNC_WAIT)
	      if(++Sync_cnt[Mod_id][CH_index] > SYNC_OK_TRESHOLD){
		Sync_status[Mod_id][CH_index]=SYNC_OK;
		Sync_cnt[Mod_id][CH_index]=0;
		msg("______________[NODE %d][PHY_EMULATION] Frame %d: PHY_SYNCHRONIZATION TO CH %d[%d]____________\n",
		    NODE_ID[Mod_id],mac_xface->frame,CH_index,NODE_ID[Mod_id]);
	      }
	    if(Sync_status[Mod_id][CH_index]==SYNC_OK)
	      mac_xface->macphy_data_ind(Mod_id,&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx,CHBCH
					   ,Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);  
	    
	  }
	  else if(Sync_status[Mod_id][CH_index]==SYNC_OK){
	    if(++Sync_cnt[Mod_id][CH_index] > SYNC_NOK_TRESHOLD){
	      msg("________________[NODE %d][PHY_EMULATION]Frame %d: PHY_SYNCHRONIZATION TO CH %d[%d] LOST, INDICATE TO RRC_____\n",
		  NODE_ID[Mod_id],mac_xface->frame,CH_index,NODE_ID[Mod_id]);
	      mac_xface->out_of_sync_ind(Mod_id,CH_index);
	      Sync_status[Mod_id][CH_index]=SYNC_WAIT;
	      Sync_cnt[Mod_id][CH_index]=0;
	    }
	  }
	  else if(Sync_status[Mod_id][CH_index]==SYNC_WAIT)
		Sync_cnt[Mod_id][CH_index]=0;
	}
	else
	  mac_xface->macphy_data_ind(Mod_id,&Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_rx
				     ,Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type
				     ,Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Lchan_id.Index);
	Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active=0;
	Macphy_req_table[Mod_id].Macphy_req_cnt = (Macphy_req_table[Mod_id].Macphy_req_cnt - 1)%NB_REQ_MAX;
      }
}

/****************************************************************************************************/
unsigned int emul_rx_handler(u8 Mode,char *rx_buffer, unsigned int Nbytes){
  /****************************************************************************************************/
  unsigned short i,Mod_id,Src_id;
  unsigned short Freq,Time,k;
  MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry=NULL;
  MACPHY_REQ_ENTRY_KEY Search_key;
  Search_key.Key_type=PHY_RESOURCES_KEY;
  unsigned int R_idx=0,CR_idx=0;
  unsigned short Rx_size, ch_index;

 

  memcpy(&Rx_size,(unsigned short*)&rx_buffer[CR_idx],sizeof(unsigned short));
  CR_idx+=sizeof(unsigned short);
 
  memcpy(&Src_id,(unsigned short*)&rx_buffer[CR_idx],sizeof(unsigned short));
  CR_idx+=sizeof(unsigned short);

  memcpy(&Freq,(unsigned short*)&rx_buffer[CR_idx],sizeof(unsigned short));
  CR_idx+=sizeof(unsigned short);
 
  Time=rx_buffer[CR_idx++];
  
  memcpy(&ch_index,(unsigned short*)&rx_buffer[CR_idx],sizeof(unsigned short));
  CR_idx+=sizeof(unsigned short);
 

  Search_key.CH_index=ch_index;
  Search_key.Key.Phy_resources.Freq_alloc=Freq;
  Search_key.Key.Phy_resources.Time_alloc=Time;





  for(Mod_id=0;Mod_id<NB_INST;Mod_id++){    //update Rssi_meas for local nodes  

    for(k=0;k<NUMBER_OF_FREQUENCY_GROUPS;k++)
      if((Freq >> k) & ONE){
	//	if(F_k0==0 && k >0) F_k0=k;
	//			msg("update [%d][%d][%d], RSSI=%d, SINR=%d\n",Emul_idx[j],Emul_idx[Mod_id],k
	//	    ,Rssi[Emul_idx[j]][Emul_idx[Mod_id]][k],Sinr[Emul_idx[j]][k]);
	//		Sinr[Emul_idx[j]][k][Time]=SAT_ADD_FIX(Sinr[Emul_idx[j]][k][Time],Rssi[Emul_idx[j]][Emul_idx[Mod_id]][k]);
	
	Rssi_meas[Mod_id][k][Time]=SAT_ADD_FIX(Rssi_meas[Mod_id][k][Time],Rssi[Emul_idx[Mod_id]][Src_id][k]);////
	/*    msg("[NODE %d][UPDATE LOCAL MEASUREMENT] Node %d TX @ Time_alloc %x, FREQ_alloc %x, Rssi_meas %d, RSSI %d\n",
	      NODE_ID[j],NODE_ID[Mod_id],
	      Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Time_alloc,
	      Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Phy_resources->Freq_alloc,
	      Rssi_meas[j][k][Time],Rssi[Emul_idx[j]][Emul_idx[Mod_id]][k]);
	*/		
      }


    //find if the PDU is to be decoded later
    Macphy_data_req_entry = find_data_req_entry(Mod_id,&Search_key);

    if (Macphy_data_req_entry) {
      
      //STORE PDU, BUT DO NOT DECODE (until we receive all remote PDUs)

      Macphy_data_req_entry->Macphy_data_req.Src_id=Src_id;
      R_idx=CR_idx;
      switch(Macphy_data_req_entry->Macphy_data_req.Pdu_type){
	//case CHBCH_SCH:
	//	if(NODE_ID[Mod_id]>7){
	// msg("PHY_SYNC\n");
	//  mac_xface->chbch_phy_sync_success(Mod_id,Macphy_data_req_entry->Macphy_data_req.CH_index);
	//}
	//	break;
      case CHBCH:
	R_idx+=deserialize_chbch(&rx_buffer[R_idx],Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Chbch_pdu);

#ifdef DEBUG_REMOTE_RX
	msg("[OPENAIR][PHY][EMUL]TTI %d: ////////////////// Inst %d Detected a CHBCH, deserializing\\\\\\\\\\\\\\\\\\\\\\\\\n",
	    mac_xface->frame,Mod_id);
#endif //DEBUG_REMOTE_RX	

	break;

      case RACH:
#ifdef DEBUG_REMOTE_RX
	msg("[OPENAIR][PHY][EMUL]TTI %d: //// Node %d Detected RACH, from SRC %d to dest %d\\\\\\\\\\\\n",
	    mac_xface->frame,
	    NODE_ID[Mod_id],
	    Src_id,
	    Search_key.CH_index);
#endif //DEBUG_REMOTE_RX	
     
	memcpy(&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Rach_pdu->Pdu_size
	       ,(unsigned short*)&rx_buffer[R_idx],USHORT_SIZE);
	R_idx+=USHORT_SIZE;
	
	memcpy(Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Rach_pdu->Rach_payload,&rx_buffer[R_idx],
	       Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Rach_pdu->Pdu_size);
	R_idx+= Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.Rach_pdu->Pdu_size;

	break;

      case UL_SACH:

#ifdef DEBUG_REMOTE_RX
	msg("RADIO_EMULATION_RX frame %d, UL_SACH on LCHAN_ID %d\n",mac_xface->frame,Macphy_data_req_entry->Macphy_data_req.Lchan_id.Index);
#endif //DEBUG_REMOTE_RX	
	R_idx+=deserialize_UL_sach(&rx_buffer[R_idx],&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.UL_sach_pdu);
	break;
	
      case DL_SACH:

#ifdef DEBUG_REMOTE_RX
	msg("[PHY_EMULATION]: DECTECTING DL_SACH_PDU....\n");
#endif //DEBUG_REMOTE_RX	
	R_idx+=deserialize_DL_sach(&rx_buffer[R_idx],&Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Pdu.DL_sach_pdu);

	break;
      }
      //Mark the PDU as it is ready to be decoded (BLER computation)
      Macphy_data_req_entry->Macphy_data_req.Direction = RX_READY;
    }
 
  }
  
  return (Rx_size+2);  
  
}



/***************************************************************************************************/
void serialize(MACPHY_DATA_REQ_TX *Tx_phy_pdu,char *Tx_buffer,unsigned int *Nbytes,unsigned char Pdu_type){
  /***************************************************************************************************/
  
  unsigned int Nbytes_tx;
  unsigned char i;
  int j;

  Nbytes_tx=*Nbytes;
  switch(Pdu_type){
  case CHBCH: 
    Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->Num_bytes_bcch;
    Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->Num_bytes_ccch;
    Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->Num_dl_sach;
    for(i=0;i<Tx_phy_pdu->Pdu.Chbch_pdu->Num_dl_sach;i++){
      memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.Chbch_pdu->DL_sacch_pdu[i].Lchan_id.Index,USHORT_SIZE);
      Nbytes_tx+=USHORT_SIZE;
      Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->DL_sacch_pdu[i].Nb_tb;
      Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->DL_sacch_pdu[i].Coding_fmt;
      memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.Chbch_pdu->DL_sacch_pdu[i].Freq_alloc,USHORT_SIZE);
      Nbytes_tx+=USHORT_SIZE;
      Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->DL_sacch_pdu[i].DL_sacch_fb.Pc;
      Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->DL_sacch_pdu[i].DL_sacch_fb.Ack;
    }
    
    Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->Num_ul_sach;
    for(i=0;i<Tx_phy_pdu->Pdu.Chbch_pdu->Num_ul_sach;i++){
      Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->UL_alloc_pdu[i].Nb_tb;
      Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.Chbch_pdu->UL_alloc_pdu[i].Coding_fmt;
      memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.Chbch_pdu->UL_alloc_pdu[i].Freq_alloc,USHORT_SIZE);
      Nbytes_tx+=USHORT_SIZE;
      memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.Chbch_pdu->UL_alloc_pdu[i].Lchan_id.Index,USHORT_SIZE);
      Nbytes_tx+=USHORT_SIZE;
    }
    memcpy(&Tx_buffer[Nbytes_tx],(char*)Tx_phy_pdu->Pdu.Chbch_pdu->Bcch_payload,Tx_phy_pdu->Pdu.Chbch_pdu->Num_bytes_bcch);  
    Nbytes_tx+=Tx_phy_pdu->Pdu.Chbch_pdu->Num_bytes_bcch;
    memcpy(&Tx_buffer[Nbytes_tx],(char*)Tx_phy_pdu->Pdu.Chbch_pdu->Ccch_payload,Tx_phy_pdu->Pdu.Chbch_pdu->Num_bytes_ccch);  
    Nbytes_tx+=Tx_phy_pdu->Pdu.Chbch_pdu->Num_bytes_ccch;  

    break;
  case DL_SACH:
    memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.DL_sach_pdu.Pdu_size,USHORT_SIZE);
    Nbytes_tx+=USHORT_SIZE;
    memcpy(&Tx_buffer[Nbytes_tx],Tx_phy_pdu->Pdu.DL_sach_pdu.Sach_payload,Tx_phy_pdu->Pdu.DL_sach_pdu.Pdu_size);
    Nbytes_tx+=Tx_phy_pdu->Pdu.DL_sach_pdu.Pdu_size;
    break;
  case UL_SACH:
    memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.Lchan_id.Index,USHORT_SIZE);
    Nbytes_tx+=USHORT_SIZE;
    memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.Pdu_size,USHORT_SIZE);
    Nbytes_tx+=USHORT_SIZE;
    Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.Pc;
    Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.Qdepth;
    memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.Ack,USHORT_SIZE);
    Nbytes_tx+=USHORT_SIZE;
    memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.cqi,UINT_SIZE);
    Nbytes_tx+=UINT_SIZE;
    Tx_buffer[Nbytes_tx++]=Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.UL_sacch_fb.Wideband_sinr;
    memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.UL_sach_pdu.Sach_payload[0],Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.Pdu_size);
    Nbytes_tx+=Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.Pdu_size;
#ifdef DUMP_EMUL_TX
        msg("[PHY_EMULATION]UL_SACH TX PDU of Size %d:\n",Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.Pdu_size);
	for (j=0;j<Tx_phy_pdu->Pdu.UL_sach_pdu.UL_sacch_pdu.Pdu_size;j++)
	  msg("%x.",Tx_phy_pdu->Pdu.UL_sach_pdu.Sach_payload[j]);
	msg("\n");
#endif //DUMP_EMUL_TX
    break;
  case RACH:
    memcpy(&Tx_buffer[Nbytes_tx],(char*)&Tx_phy_pdu->Pdu.Rach_pdu.Pdu_size,USHORT_SIZE);
    Nbytes_tx+=USHORT_SIZE;
    memcpy(&Tx_buffer[Nbytes_tx],Tx_phy_pdu->Pdu.Rach_pdu.Rach_payload,Tx_phy_pdu->Pdu.Rach_pdu.Pdu_size);
    Nbytes_tx+=Tx_phy_pdu->Pdu.Rach_pdu.Pdu_size;
    break;
  }
  (*Nbytes)=Nbytes_tx;  
}



/****************************************************************************************************************/
unsigned short deserialize_UL_sach(char *Phy_payload, UL_SACH_PDU *UL_SACH_pdu){
  unsigned short Nbytes_rx=0,i;
  memcpy(&UL_SACH_pdu->UL_sacch_pdu.Lchan_id.Index,(unsigned short*)&Phy_payload[Nbytes_rx],USHORT_SIZE);
  Nbytes_rx+=USHORT_SIZE;
  memcpy(&UL_SACH_pdu->UL_sacch_pdu.Pdu_size,(unsigned short*)&Phy_payload[Nbytes_rx],USHORT_SIZE);
  Nbytes_rx+=USHORT_SIZE;
  UL_SACH_pdu->UL_sacch_pdu.UL_sacch_fb.Pc=Phy_payload[Nbytes_rx++];
  UL_SACH_pdu->UL_sacch_pdu.UL_sacch_fb.Qdepth=Phy_payload[Nbytes_rx++];
  memcpy(&UL_SACH_pdu->UL_sacch_pdu.UL_sacch_fb.Ack,(unsigned short*)&Phy_payload[Nbytes_rx],USHORT_SIZE);
  Nbytes_rx+=USHORT_SIZE;
  memcpy(&UL_SACH_pdu->UL_sacch_pdu.UL_sacch_fb.cqi,(unsigned int*)&Phy_payload[Nbytes_rx],UINT_SIZE);
  Nbytes_rx+=UINT_SIZE;
  UL_SACH_pdu->UL_sacch_pdu.UL_sacch_fb.Wideband_sinr=Phy_payload[Nbytes_rx++];
  memcpy(&UL_SACH_pdu->Sach_payload,&Phy_payload[Nbytes_rx],UL_SACH_pdu->UL_sacch_pdu.Pdu_size);
  Nbytes_rx+=UL_SACH_pdu->UL_sacch_pdu.Pdu_size;
#ifdef DUMP_EMUL_RX
  msg("[PHY_EMULATION]RX_PDU_SIZE=%d\n",Nbytes_rx);
  for (i=0;i<Nbytes_rx;i++)
    msg("%x.",UL_SACH_pdu->Sach_payload[i]);
  msg("\n");
#endif //DUMP_EMUL_RX
  return Nbytes_rx;
}
/****************************************************************************************************************/
unsigned short deserialize_DL_sach(char *Phy_payload, DL_SACH_PDU *DL_SACH_pdu){
  unsigned short Nbytes_rx=0;
  memcpy(&DL_SACH_pdu->Pdu_size,(unsigned short*)&Phy_payload[Nbytes_rx],USHORT_SIZE);
  Nbytes_rx+=USHORT_SIZE;
  memcpy(DL_SACH_pdu->Sach_payload,&Phy_payload[Nbytes_rx],DL_SACH_pdu->Pdu_size);
  Nbytes_rx+=DL_SACH_pdu->Pdu_size;
  return Nbytes_rx;
}


/****************************************************************************************************************/
unsigned short deserialize_chbch(char *Phy_payload,CHBCH_PDU* CHbch_pdu){

  unsigned char i;
  unsigned short Nrx=0;
  CHbch_pdu->Num_bytes_bcch=Phy_payload[Nrx++];
  CHbch_pdu->Num_bytes_ccch=Phy_payload[Nrx++];
  CHbch_pdu->Num_dl_sach=Phy_payload[Nrx++];
  for(i=0;i<CHbch_pdu->Num_dl_sach;i++){
    memcpy(&CHbch_pdu->DL_sacch_pdu[i].Lchan_id.Index,(unsigned short*)&Phy_payload[Nrx],USHORT_SIZE);
    Nrx+=USHORT_SIZE;
    CHbch_pdu->DL_sacch_pdu[i].Nb_tb=Phy_payload[Nrx++];
    CHbch_pdu->DL_sacch_pdu[i].Coding_fmt=Phy_payload[Nrx++];
    memcpy(&CHbch_pdu->DL_sacch_pdu[i].Freq_alloc,(unsigned short*)&Phy_payload[Nrx],USHORT_SIZE);
    Nrx+=USHORT_SIZE;
    CHbch_pdu->DL_sacch_pdu[i].DL_sacch_fb.Pc=Phy_payload[Nrx++];
    CHbch_pdu->DL_sacch_pdu[i].DL_sacch_fb.Ack=Phy_payload[Nrx++];
  }
  CHbch_pdu->Num_ul_sach=Phy_payload[Nrx++];
  for(i=0;i<CHbch_pdu->Num_ul_sach;i++){
    CHbch_pdu->UL_alloc_pdu[i].Nb_tb=Phy_payload[Nrx++];
    CHbch_pdu->UL_alloc_pdu[i].Coding_fmt=Phy_payload[Nrx++];
    memcpy(&CHbch_pdu->UL_alloc_pdu[i].Freq_alloc,(unsigned short*)&Phy_payload[Nrx],USHORT_SIZE);
    Nrx+=USHORT_SIZE;
    memcpy(&CHbch_pdu->UL_alloc_pdu[i].Lchan_id.Index,(unsigned short*)&Phy_payload[Nrx],USHORT_SIZE);
    Nrx+=USHORT_SIZE;
  }
  memcpy(CHbch_pdu->Bcch_payload,&Phy_payload[Nrx],CHbch_pdu->Num_bytes_bcch);
  Nrx+=CHbch_pdu->Num_bytes_bcch;
  memcpy(CHbch_pdu->Ccch_payload,&Phy_payload[Nrx],CHbch_pdu->Num_bytes_ccch);
  Nrx+=CHbch_pdu->Num_bytes_ccch;
  return Nrx;
}

/******************************************************************************************************/ 

  /******************************************************************************************************/ 
  /*  unsigned short i,Mod_id;
  for(Mod_id=0;Mod_id<NB_INST;Mod_id++)
    for(i=0;i<NB_REQ_MAX;i++){
      if(Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active ==1
	 && Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type == SCH){
	phy_meas_ul_sch(Mod_id,&Macphy_req_table[Mod_id].Macphy_req_table_entry[i]);
	mac_xface->macphy_data_ind(Mod_id,&Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_rx,
				   UL_SCH,Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Lchan_id.Index);
	Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active=0;
	Macphy_req_table[Mod_id].Macphy_req_cnt = (Macphy_req_table[Mod_id].Macphy_req_cnt - 1)%NB_REQ_MAX;	 
      }
    }
  */


/******************************************************************************************************/ 
void emul_meas_ul_sch(void){
  /******************************************************************************************************/ 


  unsigned short Src_id;
  unsigned short k,Mod_id,i;

  for(Mod_id=0;Mod_id<NB_INST;Mod_id++)
    for(i=0;i<NB_REQ_MAX;i++){
      if( (Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active ==1)
	  && (Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Direction == RX) 
	  && (Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type >= DL_SCH)  ){

	Src_id=Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_rx.Src_id;
	for (k=0;k<NB_NODE;k++)
	  if(NODE_LIST[k]==Src_id){
	    Src_id=k;
	    break;
	  } 
	for(k=0;k<NUMBER_OF_FREQUENCY_GROUPS;k++){
	  if( Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type == UL_SCH )
	    Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Sub_band_sinr[k]
	      = dB_fixed(Rssi[Emul_idx[Mod_id]][Src_id][k]);
	  else
	    Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_rx.Meas.DL_meas->Sub_band_sinr[k]
	      = dB_fixed(Rssi[Emul_idx[Mod_id]][Src_id][k]);
	}
	Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_rx.crc_status[i]=0;
	mac_xface->macphy_data_ind(Mod_id,&Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_rx,
				   Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type, 
				   Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Macphy_data_req.Lchan_id.Index);  
	Macphy_req_table[Mod_id].Macphy_req_table_entry[i].Active=0;
	Macphy_req_table[Mod_id].Macphy_req_cnt = (Macphy_req_table[Mod_id].Macphy_req_cnt - 1)%NB_REQ_MAX;	 
	
	  /*	  int Rssi_inst,I0_inst;
	  Rssi_inst=0;
	  I0_inst=0;
	  for(k=0;k<NUMBER_OF_FREQUENCY_GROUPS;k++){
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Sub_band_sinr[k]=dB_fixed(Rssi[Emul_idx[Mod_id]][Src_id][k]);    
	  Rssi_inst=SAT_ADD_FIX(Rssi_inst,Rssi[Emul_idx[Mod_id]][Src_id][k]);
	  I0_inst+=Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Sub_band_sinr[k];
	  }
	  //     msg("RSSI = %d,(%ddB), I+N0 =%d,(%d dB)\n",Rssi_inst,dB_fixed(Rssi_inst),I0_inst,dB_fixed(I0_inst));
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Wideband_rssi_dBm=dB_fixed(Rssi_inst);
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.Meas.UL_meas->Wideband_interference_level_dBm=I0_inst;
	  Macphy_data_req_entry->Macphy_data_req.Dir.Req_rx.crc_status[0]=1;
	  }
	  */
      }
    }
}
