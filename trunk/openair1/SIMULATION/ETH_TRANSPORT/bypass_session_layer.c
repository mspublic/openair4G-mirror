/*! \file bypass_session_layer.h
* \brief implementation of emultor tx and rx 
* \author Navid Nikaein and Raymomd Knopp
* \date 2011
* \version 1.0 
* \company Eurecom
* \email: navid.nikaein@eurecom.fr
*/ 

#include "PHY/defs.h"
#include "defs.h"
#include "extern.h"
//#include "mac_extern.h"

#include "UTIL/LOG/log_if.h"

#ifdef USER_MODE
#include "multicast_link.h"
#endif

/***************************************************************************/
char rx_bufferP[BYPASS_RX_BUFFER_SIZE];
static unsigned int num_bytesP=0;
int N_P=0,N_R=0;
char     bypass_tx_buffer[BYPASS_TX_BUFFER_SIZE];
unsigned int Master_list_rx, Seq_nb;
/***************************************************************************/


/***************************************************************************/
void bypass_init ( int (*tx_handlerP) (unsigned char,char*, unsigned int*, unsigned int*),int (*rx_handlerP) (unsigned char,char*,int)){
/***************************************************************************/
#ifdef USER_MODE
  multicast_link_start (bypass_rx_handler, emu_info.multicast_group);
#endif //USER_MODE
  tx_handler = tx_handlerP;
  rx_handler = rx_handlerP;
  Master_list_rx=0;
  emu_tx_status = WAIT_SYNC_TRANSPORT;
  emu_rx_status = WAIT_SYNC_TRANSPORT;
}
/***************************************************************************/
int bypass_rx_data (unsigned int last_slot, unsigned int next_slot){
/***************************************************************************/
  bypass_msg_header_t *messg;
  bypass_proto2multicast_header_t *bypass_read_header;
  eNB_transport_info_t *eNB_info;
  UE_transport_info_t  *UE_info;
  int             tmp_byte_count;
  int             bytes_read = 0;
  int             bytes_data_to_read;
  int             num_flows;
  int             current_flow; 
  int             m_id, n_enb, n_ue, n_dci, total_tbs=0, total_header=0;

  pthread_mutex_lock(&emul_low_mutex);
  if(emul_low_mutex_var){
    LOG_T(EMU, " WAIT BYPASS_PHY...\n");
    pthread_cond_wait(&emul_low_cond, &emul_low_mutex); 
  }
  if(num_bytesP==0){
    //msg("[BYPASS] IDLE_WAIT\n");
    //exit(0);
    pthread_mutex_unlock(&emul_low_mutex);
  }
  else{
    LOG_T(EMU,"BYPASS_RX_DATA: IN, Num_bytesp=%d...\n",num_bytesP);
    bypass_read_header = (bypass_proto2multicast_header_t *) (&rx_bufferP[bytes_read]);
    bytes_read += sizeof (bypass_proto2multicast_header_t);
    bytes_data_to_read = bypass_read_header->size;
    if(num_bytesP!=bytes_read+bytes_data_to_read) {
      LOG_W(EMU, "WARNINIG BYTES2READ # DELIVERED BYTES!!!\n");
    }
    else{
      messg = (bypass_msg_header_t *) (&rx_bufferP[bytes_read]);
      bytes_read += sizeof (bypass_msg_header_t);
      LOG_I(EMU, "message type is %d and last slot %d\n", messg->Message_type, messg->last_slot);
      //sleep(1);//eNB_info = (eNB_transport_info_t *) (&rx_bufferP[bytes_read]);
      //chek if MASTER in my List
      // switch(Emulation_status){
      switch(messg->Message_type){	
	//case WAIT_SYNC_TRANSPORT:
      
      case WAIT_TRANSPORT_INFO:
	Master_list_rx=((Master_list_rx) |(1<< messg->master_id));
	break;
      case SYNC_TRANSPORT_INFO:
	
	// determite the total number of remote enb & ue 
	emu_info.nb_enb_remote += messg->nb_enb;
	emu_info.nb_ue_remote += messg->nb_ue;
	// determine the index of local enb and ue wrt the remote ones  
	if (  messg->master_id < emu_info.master_id ){
	  emu_info.first_enb_local +=messg->nb_enb;
	  emu_info.first_ue_local +=messg->nb_ue;
	}
	
	// store param for enb per master
	if ((emu_info.master[messg->master_id].nb_enb = messg->nb_enb) > 0 ){
	  for (m_id=0;m_id < messg->master_id; m_id++ ){
	    emu_info.master[messg->master_id].first_enb+=emu_info.master[m_id].nb_enb;
	  }
	  LOG_T(EMU, "WAIT_SYNC_TRANSPORT state:  for master %d the first enb index is %d\n",
		messg->master_id, emu_info.master[messg->master_id].first_enb);	  
	}
	// store param fo ue per master
	if ((emu_info.master[messg->master_id].nb_ue  = messg->nb_ue) > 0){
	  for (m_id=0;m_id < messg->master_id; m_id++ ){
	    emu_info.master[messg->master_id].first_ue+=emu_info.master[m_id].nb_ue;
	  }
	  LOG_T(EMU, "WAIT_SYNC_TRANSPORT state: for master %d the first ue index is %d\n",
		messg->master_id, emu_info.master[messg->master_id].first_ue);	
	}      
	
	Master_list_rx=((Master_list_rx) |(1<< messg->master_id));
	if (Master_list_rx == emu_info.master_list) {
	  emu_rx_status = SYNCED_TRANSPORT;
	}
	LOG_T(EMU,"WAIT_SYNC_TRANSPORT state: m_id %d total enb remote %d total ue remote %d first enb index %d first ue index %d emu_rx_status %d\n", 
	     messg->master_id,   emu_info.nb_enb_remote, emu_info.nb_ue_remote,
	      emu_info.first_enb_local, emu_info.first_ue_local, emu_rx_status  );

	break;

	//case WAIT_ENB_TRANSPORT:
      case ENB_TRANSPORT_INFO:
	clear_UE_transport_info(emu_info.nb_ue_local+emu_info.nb_ue_remote);
	LOG_T(EMU,"RX eNB TRANSPORT INFO master id %d nb_ue %d \n", 
	      messg->master_id,emu_info.master[messg->master_id].nb_enb);
	if (emu_info.master[messg->master_id].nb_enb > 0 ){
	  total_header=0;
	  total_tbs=0;
	  total_header += sizeof(eNB_transport_info_t)-MAX_TRANSPORT_BLOCKS_BUFFER_SIZE;
	  
	  eNB_info = (eNB_transport_info_t *) (&rx_bufferP[bytes_read]);
	  for (n_enb = emu_info.master[messg->master_id].first_enb; 
	       n_enb < emu_info.master[messg->master_id].nb_enb ;
	       n_enb ++) 
	    for (n_dci = 0 ; 
		 n_dci < (eNB_info[n_enb].num_ue_spec_dci+eNB_info[n_enb].num_common_dci);
		 n_dci ++) 
	      total_tbs+=eNB_info[n_enb].tbs[n_dci];
	    
	    memcpy (&eNB_transport_info,
		    eNB_info,
		    total_header+total_tbs);
	    bytes_read+=total_header+total_tbs;
	    LOG_T(EMU,"RX eNB Transport buffer total size %d (header%d,tbs %d) \n",
		  total_header+total_tbs, total_header,total_tbs);

	    for (n_enb = emu_info.master[messg->master_id].first_enb; 
		 n_enb < emu_info.master[messg->master_id].nb_enb ;
		 n_enb ++) 
	      fill_phy_enb_vars(n_enb,last_slot,next_slot);
	}
	else{
	  LOG_T(EMU,"WAIT_ENB_TRANSPORT state: no enb transport info from master %d \n", messg->master_id);
	}

	Master_list_rx=((Master_list_rx) |(1<< messg->master_id));
	if (Master_list_rx == emu_info.master_list) {
	  emu_rx_status = SYNCED_TRANSPORT;
	}	
	break;
	
      case UE_TRANSPORT_INFO:
	clear_eNB_transport_info(emu_info.nb_enb_local+emu_info.nb_enb_remote);
	LOG_T(EMU,"RX UE_TRANSPORT_INFO master id %d nb_ue %d \n", 
	      messg->master_id, emu_info.master[messg->master_id].nb_ue);
	
	if (emu_info.master[messg->master_id].nb_ue > 0 ){
	  // get the header first 
	  total_header=0;
	  total_tbs=0;
	  total_header += sizeof(UE_transport_info_t)-MAX_TRANSPORT_BLOCKS_BUFFER_SIZE;
	  UE_info = (UE_transport_info_t *) (&rx_bufferP[bytes_read]);
	  // get the total size of the transport blocks
	  for (n_ue = emu_info.master[messg->master_id].first_ue; 
	       n_ue < emu_info.master[messg->master_id].nb_ue ;
	       n_ue ++) 
	    for (n_enb = 0;n_enb < UE_info[n_ue].num_eNB; n_enb ++) 
	      total_tbs+=UE_info[n_ue].tbs[n_enb];
	  
	  memcpy (&UE_transport_info,
		  UE_info,
		  total_header+total_tbs);
	  bytes_read+=total_header+total_tbs;
	  
	  LOG_T(EMU,"RX Total size of buffer is %d (header%d,tbs %d) \n",
		total_header+total_tbs,total_header,total_tbs);

	  for (n_enb=0; n_enb < UE_info[0].num_eNB; n_enb ++ )
	    LOG_T(EMU,"dump ue transport info rnti %x enb_id %d, harq_id %d tbs %d\n", 
		  UE_transport_info[0].rnti[n_enb],
		  UE_transport_info[0].eNB_id[n_enb],
		  UE_transport_info[0].harq_pid[n_enb],
		  UE_transport_info[0].tbs[n_enb]);

	  for (n_ue = emu_info.master[messg->master_id].first_ue; 
	       n_ue < emu_info.master[messg->master_id].nb_ue ;
	       n_ue ++) {
	    fill_phy_ue_vars(n_ue,last_slot);
	  }
	}
	else{
	  LOG_T(EMU,"WAIT_UE_TRANSPORT state: no UE transport info from master %d\n", messg->master_id );
	}
	
	Master_list_rx=((Master_list_rx) |(1<< messg->master_id));
	if (Master_list_rx == emu_info.master_list) {
	  emu_rx_status = SYNCED_TRANSPORT;
	}
	break;
      case RELEASE_TRANSPORT_INFO :
	Master_list_rx == emu_info.master_list;
	LOG_E(EMU, "RX RELEASE_TRANSPORT_INFO\n");
	  break;
      default:
	msg("[MAC][BYPASS] ERROR RX UNKNOWN MESSAGE\n");    
	//mac_xface->macphy_exit("");
	break;
      }
    }
  
    num_bytesP=0;
    emul_low_mutex_var=1; 
    //msg("[BYPASS] CALLING_SIGNAL_HIGH_MAC\n");
    pthread_cond_signal(&emul_low_cond);
    pthread_mutex_unlock(&emul_low_mutex);
    bypass_signal_mac_phy(last_slot,next_slot);


  }


  return bytes_read;
}

/******************************************************************************************************/ 
#ifndef USER_MODE 
int bypass_rx_handler(unsigned int fifo, int rw){
 /******************************************************************************************************/ 
  //  if(rw=='w'){
    int             bytes_read;
    int             bytes_processed=0;
    int             header_bytes; //, elapsed_time;
    //printk("[BYPASS] BYPASS_RX_HANDLER IN...\n");
    header_bytes= rtf_get(fifo_bypass_phy_user2kern, rx_bufferP,sizeof(bypass_proto2multicast_header_t) );
    if (header_bytes> 0) {
      bytes_read = rtf_get(fifo_bypass_phy_user2kern, &rx_bufferP[header_bytes],((bypass_proto2multicast_header_t *) (&rx_bufferP[0]))->size);
      // printk("BYTES_READ=%d\n",bytes_read);
      if (bytes_read > 0) {
	num_bytesP=header_bytes+bytes_read;
	emul_low_mutex_var=0;
	//printk("BYPASS_PHY SIGNAL MAC_LOW...\n");
	pthread_cond_signal(&emul_low_cond);
      }
    }
    // }
  return 0;
}
#else //USER_MODE
/******************************************************************************************************/ 
void bypass_rx_handler(unsigned int Num_bytes,char *Rx_buffer){
/******************************************************************************************************/ 
//  msg("[BYPASS] BYPASS RX_HANDLER IN ...\n");
  if(Num_bytes >0){
    pthread_mutex_lock(&emul_low_mutex);
    while(!emul_low_mutex_var){
      //    msg("[BYPASS] BYPASS: WAIT MAC_LOW...\n");
      pthread_cond_wait(&emul_low_cond, &emul_low_mutex); 
    }
    num_bytesP=Num_bytes;
    memcpy(rx_bufferP,Rx_buffer,Num_bytes);
    emul_low_mutex_var=0;
    //msg("[BYPASS] RX_HANDLER SIGNAL MAC_LOW\n");
    pthread_cond_signal(&emul_low_cond); //on ne peut que signaler depuis un context linux (rtf_handler); pas de wait, jamais!!!!!!
    pthread_mutex_unlock(&emul_low_mutex);
  }
}
#endif //USER_MODE

/******************************************************************************************************/ 
void  bypass_signal_mac_phy(unsigned int last_slot, unsigned int next_slot){
/******************************************************************************************************/ 
  char tt=1;   

  if(Master_list_rx != emu_info.master_list){
#ifndef USER_MODE
    rtf_put(fifo_mac_bypass,&tt,1);  // the Rx window is still opened  (Re)signal bypass_phy (emulate MAC signal)  
#endif //USER_MODE      
    bypass_rx_data(last_slot,next_slot);
  }
  else Master_list_rx=0;
}

#ifndef USER_MODE
/***************************************************************************/
int multicast_link_write_sock (int groupP, char *dataP, unsigned int sizeP){
/***************************************************************************/
  int             tx_bytes=0;
   
  pthread_mutex_lock(&Tx_mutex);  
  while(!Tx_mutex_var){
    //msg("[BYPASS]RG WAIT USER_SPACE FIFO SIGNAL..\n");
    pthread_cond_wait(&Tx_cond,&Tx_mutex);
  }
  Tx_mutex_var=0;
  N_P=(int)((sizeP-sizeof (bypass_proto2multicast_header_t))/1000)+2;
  tx_bytes += rtf_put (fifo_bypass_phy_kern2user, &dataP[tx_bytes],sizeof (bypass_proto2multicast_header_t));
  while(tx_bytes<sizeP){
    if(sizeP-tx_bytes<=1000)
      tx_bytes += rtf_put (fifo_bypass_phy_kern2user, &dataP[tx_bytes],sizeP-tx_bytes);
    else
      tx_bytes += rtf_put (fifo_bypass_phy_kern2user, &dataP[tx_bytes],1000);
  }
  //RG_tx_mutex_var=0;
  pthread_mutex_unlock(&Tx_mutex);
  
  return tx_bytes;
}
#endif

/***************************************************************************/
void bypass_tx_data(char Type, unsigned int last_slot){
  /***************************************************************************/
  unsigned int         num_flows;
  bypass_msg_header_t *messg;
  unsigned int         byte_tx_count;
  eNB_transport_info_t *eNB_info;
  int n_enb,n_ue, n_dci,total_tbs=0,total_size=0;
  messg = (bypass_msg_header_t *) (&bypass_tx_buffer[sizeof (bypass_proto2multicast_header_t)]);
  num_flows = 0;
  messg->master_id       = emu_info.master_id; //Master_id;
  //  messg->nb_master       = emu_info.nb_master;
  messg->nb_enb          = emu_info.nb_enb_local; //Master_id;
  messg->nb_ue           = emu_info.nb_ue_local; //Master_id;
  messg->nb_flow         = num_flows;
  messg->last_slot       = last_slot;
  byte_tx_count = sizeof (bypass_msg_header_t) + sizeof (bypass_proto2multicast_header_t);
  
  if(Type==WAIT_TRANSPORT){
    messg->Message_type = WAIT_TRANSPORT_INFO;
    LOG_T(EMU,"[TX_DATA] WAIT SYNC TRANSPORT\n");
  }
  else if(Type==SYNC_TRANSPORT){
    messg->Message_type = SYNC_TRANSPORT_INFO;
    LOG_T(EMU,"[TX_DATA] SYNC TRANSPORT\n");
  }
  else if(Type==ENB_TRANSPORT){
    messg->Message_type = ENB_TRANSPORT_INFO;
    //memcpy(&bypass_tx_buffer[byte_tx_count], (char*)eNB_transport_info, sizeof(eNB_transport_info_t));
    //byte_tx_count +=sizeof(eNB_transport_info_t);
     total_size=0;
     total_tbs=0;
    for (n_enb=emu_info.first_enb_local;n_enb<(emu_info.first_enb_local+emu_info.nb_enb_local);n_enb++) 
      for (n_dci =0 ; 
	   n_dci < (eNB_transport_info[n_enb].num_ue_spec_dci+ eNB_transport_info[n_enb].num_common_dci);
	   n_dci++) 
	total_tbs+=eNB_transport_info[n_enb].tbs[n_dci];
    total_size = sizeof(eNB_transport_info_t)+total_tbs-MAX_TRANSPORT_BLOCKS_BUFFER_SIZE;
    LOG_I(EMU, "TX eNB Transport Total size of buffer is %d (header%d,tbs %d)\n",
	  total_size, total_size-total_tbs, total_tbs);
    memcpy(&bypass_tx_buffer[byte_tx_count], (char*)eNB_transport_info, total_size);
    byte_tx_count +=total_size;
  }
  else if (Type == UE_TRANSPORT){ 
    messg->Message_type = UE_TRANSPORT_INFO;
    // memcpy(&bypass_tx_buffer[byte_tx_count], (char*)UE_transport_info, sizeof(UE_transport_info_t)); 
    //byte_tx_count +=total_tbs;
    total_size=0;
    total_tbs=0; // compute the actual size of transport_blocks
    for (n_ue = emu_info.first_ue_local; n_ue < (emu_info.first_ue_local+emu_info.nb_ue_local);n_ue++)
      for (n_enb=0;n_enb<UE_transport_info[n_ue].num_eNB;n_enb++) 
	total_tbs+=UE_transport_info[n_ue].tbs[n_enb];
    total_size = sizeof(UE_transport_info_t)+total_tbs-MAX_TRANSPORT_BLOCKS_BUFFER_SIZE;
    LOG_I(EMU, "TX UE Transport Total size of buffer is %d (header%d,tbs %d)\n",
	  total_size, total_size-total_tbs, total_tbs);
   
    memcpy(&bypass_tx_buffer[byte_tx_count], (char*)UE_transport_info, total_size);
    byte_tx_count +=total_size;
    
  } 
  else if (Type == RELEASE_TRANSPORT){
    messg->Message_type = RELEASE_TRANSPORT_INFO;
  }else {
    LOG_T(EMU,"[TX_DATA] UNKNOWN MSG  \n");
  }

  ((bypass_proto2multicast_header_t *) bypass_tx_buffer)->size = byte_tx_count - sizeof (bypass_proto2multicast_header_t); 
  //if(mac_xface->frame%1000==0)   
  multicast_link_write_sock (emu_info.multicast_group, bypass_tx_buffer, byte_tx_count);
}

#ifndef USER_MODE 
/*********************************************************************************************************************/
int bypass_tx_handler(unsigned int fifo, int rw){
  /***************************************************************************/
  // if(rw=='r'){
    if(++N_R==N_P){
      //msg("[OPENAIR][RG_BYPASS] TX_handler..\n");
  //    pthread_mutex_lock(&RG_tx_mutex);
      rtf_reset(fifo_bypass_phy_kern2user);
  //     pthread_mutex_lock(&RG_tx_mutex);
      Tx_mutex_var=1;
      N_R=0;
//      pthread_mutex_unlock(&RG_tx_mutex);
      pthread_cond_signal(&Tx_cond);    
    }
    // }
}
#endif

