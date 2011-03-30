/*________________________phy_emulation.c________________________

  Authors : Hicham Anouar
  Company : EURECOM
  Emails  : anouar@eurecom.fr
  ________________________________________________________________*/

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"

extern unsigned int   Master_list_rx;
extern unsigned short NODE_ID[1];
extern unsigned char  NB_INST;
#define DEBUG_CONTROL 1
/******************************************************************************************************/ 
char is_node_local_neighbor(unsigned short Node_id){
  /******************************************************************************************************/ 
  int i;
  for(i=0;i<NB_INST;i++)
    if(NODE_ID[i]==Node_id) return 1;
  return 0; 
}

/******************************************************************************************************/ 
void emulation_tx_rx(void){
  /******************************************************************************************************/ 
#ifndef USER_MODE
  char tt=1;
#endif //USER_MODE

  // This is the synchronization control packet
  if(!Is_primary_master){////more than one machine in the emulation scenario, this is not the primary master machine
   
    bypass_tx_data(CH_BYPASS_CONTROL);
    // ALL emulation masters (EM), except the primary master(PM, (machine with Id 0 in the topology script)                                   
    // send a ctrl msg to the PM to synchronize
    // note that there is one EM per machine. Pm & EMs are logical entities, independent from nodes in the 
    // emulation scenario,  used to control the exchange of MAC PDUs on the emulation medium 

    Emulation_status=WAIT_PM_CT;// then wait control msg from the PM
    Master_list_rx=Master_list-1; // e.q: waiting just for the PM
#ifdef DEBUG_CONTROL
    msg("[Emu] TX CONTROL SIGNAL TO PRIMARY MASTER\n ");
    msg("[EMULATION_CONTROL] Status %d (%d)\n",Emulation_status,WAIT_PM_CT) ; 
#endif

#ifndef USER_MODE
    rtf_put(fifo_mac_bypass,&tt,1);  
#endif //USER_MODE      
    bypass_rx_data();
#ifdef DEBUG_CONTROL
    msg("[Emu] RX CONTROL SIGNAL FROM PRIMARY MASTER\n ");
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
    msg("[Emu] PM: RX CONTROL SIGNAL FROM SECONDARY MASTERS\n ");
#endif //DEBUG_CONTROL   
    bypass_tx_data(CH_BYPASS_CONTROL);//when the PM receives ctrl msg from all EM, he sends also a CTRL msg to them
#ifdef DEBUG_CONTROL
    msg("[Emu]PM: TX CONTROL SIGNAL TO SECONDARY MASTERS\n ");
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

  
  //  emul_rx_local_ul_dl_data();//decoding local DL/UL PDUs

  emul_rx_data();//decoding remote DL/UL PDUs

  //  clear_non_ack_req();//CHBCH, RACH, and missing sach (UL MAP not decoded correctly or node loosing synchro)

#ifdef DEBUG_CONTROL
  msg("FRAME %d: EMULATION TX/RX Done \n", mac_xface->frame);
#endif  
}

/****************************************************************************************************/
unsigned int emul_tx_handler(unsigned char Mode,char *Tx_buffer,unsigned int* Nbytes,unsigned int *Nb_flows){
  /****************************************************************************************************/
  unsigned short k,Mod_id;
  
  for(k=0;k<NB_INST;k++){//Nb_out_src[Mode];k++){
    Mod_id=k;//Out_list[Mode][k];
  }  
  return *Nbytes;
}



/****************************************************************************************************/
unsigned int emul_rx_data(void){
  /****************************************************************************************************/
  return(0);
}

/****************************************************************************************************/
unsigned int emul_rx_handler(unsigned char Mode,char *rx_buffer, unsigned int Nbytes){
  /****************************************************************************************************/
  unsigned short Rx_size=0;

 

  
  return (Rx_size+2);  
  
}

void clear_eNB_transport_info(u8 nb_eNB) {
  u8 eNB_id;
  
  for (eNB_id=0;eNB_id<nb_eNB;eNB_id++) {
    eNB_transport_info_TB_index[eNB_id]=0;
    memset((void *)&eNB_transport_info[eNB_id].cntl,0,sizeof(eNB_cntl));
    eNB_transport_info[eNB_id].num_common_dci=0;
    eNB_transport_info[eNB_id].num_ue_spec_dci=0;
  }
}

void clear_UE_transport_info(u8 nb_UE) {
  u8 UE_id;
  
  for (UE_id=0;UE_id<nb_UE;UE_id++) {
    UE_transport_info_TB_index[UE_id]=0;
    memset((void *)&UE_transport_info[UE_id].cntl,0,sizeof(UE_cntl));
  }
}

