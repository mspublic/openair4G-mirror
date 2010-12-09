/*________________________bypass_session_layer.c________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/
#include "defs.h"
#include "vars.h"
//#include "mac_extern.h"

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
  multicast_link_start (bypass_rx_handler);
#endif //USER_MODE
  tx_handler = tx_handlerP;
  rx_handler = rx_handlerP;
  Master_list_rx=0;
}
/***************************************************************************/
int bypass_rx_data (void){
/***************************************************************************/
  bypass_msg_header_t *messg;
  bypass_proto2multicast_header_t *bypass_read_header;
  int             tmp_byte_count;
  int             bytes_read = 0;
  int             bytes_data_to_read;
  int             num_flows;
  int             current_flow;  
  unsigned int Wahed=1;
  pthread_mutex_lock(&Mac_low_mutex);
  if(Mac_low_mutex_var){
    //    msg("[BYPASS] WAIT BYPASS_PHY...\n");
    pthread_cond_wait(&Mac_low_cond, &Mac_low_mutex); 
  }
  if(num_bytesP==0){
    //msg("[BYPASS] IDLE_WAIT\n");
    //exit(0);
    pthread_mutex_unlock(&Mac_low_mutex);
  }
  else{
    // msg("[BYPASS] BYPASS_RX_DATA: IN, Num_bytesp=%d...\n",num_bytesP);
    bypass_read_header = (bypass_proto2multicast_header_t *) (&rx_bufferP[bytes_read]);
    bytes_read += sizeof (bypass_proto2multicast_header_t);
    bytes_data_to_read = bypass_read_header->size;
    if(num_bytesP!=bytes_read+bytes_data_to_read) {
      msg("[BYPASS] WARNINIG BYTES2READ # DELIVERED BYTES!!!\n");
    }
    else{
      messg = (bypass_msg_header_t *) (&rx_bufferP[bytes_read]);
      bytes_read += sizeof (bypass_msg_header_t);
      //chek if MASTER in my List
      switch(Emulation_status){
      case WAIT_PM_CT:
	if(messg->M_id == 0){
	  Master_list_rx=((Master_list_rx) |(Wahed<< messg->M_id));
	  //    msg("[BYPASS] RX_PRIMARY_MASTER_CONTROL_MESSAGE \n");
	}
	break;
      case WAIT_EM_CT:
	  Master_list_rx=((Master_list_rx) |(Wahed<< messg->M_id));
	  //msg("[BYPASS] RX_MASTER %d CONTROL_MESSAGE\n",messg->M_id);
	
	break;
	
	/*      case WAIT_CH_CT:
	if(!Is_primary_master){
	  if(messg->M_id == 0){
	    Master_list_rx=((Master_list_rx) |(Wahed<< messg->M_id));
	    //    msg("[BYPASS] RX_PRIMARY_MASTER_CONTROL_MESSAGE \n");
	  }
	}
	else{
	  Master_list_rx=((Master_list_rx) |(Wahed<< messg->M_id));
	  //msg("[BYPASS] RX_MASTER %d CONTROL_MESSAGE\n",messg->M_id);
	}
	break;
	*/
      case WAIT_CHBCH_DATA:
	if(messg->Message_type == BYPASS_CHBCH_DATA){
	  // msg("[BYPASS] RX_CH_DATA_MESSAGE from Master %d \n",messg->M_id);
	  Master_list_rx=((Master_list_rx) |(Wahed<< messg->M_id));
	  current_flow = 0;
	  num_flows = messg->Nb_flows;
	  //msg("[BYPASS] Nb_flows %d,num_bytesP %d, bytes_read %d, Buffer %p\n",num_flows,num_bytesP,bytes_read,rx_bufferP);
	  while ((num_bytesP > bytes_read) && (current_flow < num_flows)) {
	    tmp_byte_count = rx_handler (CH_TRAFFIC,&rx_bufferP[bytes_read],num_bytesP-bytes_read);
	    current_flow += 1;
	    bytes_read = bytes_read + tmp_byte_count;
	  }
	  break;
	}
      case WAIT_UL_DL_DATA:
	if(messg->Message_type == BYPASS_UL_DL_DATA){
	  //	  msg("[BYPASS] RX_UE_DATA_MESSAGE from Master%d \n",messg->M_id);
	  Master_list_rx=((Master_list_rx) |(Wahed<< messg->M_id));
	  current_flow = 0;
	  num_flows = messg->Nb_flows;
	  //msg("[BYPASS] Nb_flows %d,num_bytesP %d, bytes_read %d, Buffer %p\n",num_flows,num_bytesP,bytes_read,rx_bufferP);
	  while ((num_bytesP > bytes_read) && (current_flow < num_flows)) {
	    //   msg("[BYPASS] CURRENT_FLOW %d\n",current_flow);
	    tmp_byte_count = rx_handler (UE_TRAFFIC,&rx_bufferP[bytes_read],num_bytesP-bytes_read);
	    current_flow += 1;
	    bytes_read = bytes_read + tmp_byte_count;
	  }
	  break;
	}
      default:
	msg("[MAC][BYPASS] ERROR RX UNKNOWN MESSAGE\n");    
	//mac_xface->macphy_exit("");
	break;
      }
    }
  
    num_bytesP=0;
    Mac_low_mutex_var=1; 
    //msg("[BYPASS] CALLING_SIGNAL_HIGH_MAC\n");
    pthread_cond_signal(&Mac_low_cond);
    pthread_mutex_unlock(&Mac_low_mutex);
    bypass_signal_mac_phy();


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
	Mac_low_mutex_var=0;
	//printk("BYPASS_PHY SIGNAL MAC_LOW...\n");
	pthread_cond_signal(&Mac_low_cond);
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
    pthread_mutex_lock(&Mac_low_mutex);
    while(!Mac_low_mutex_var){
      //    msg("[BYPASS] BYPASS: WAIT MAC_LOW...\n");
      pthread_cond_wait(&Mac_low_cond, &Mac_low_mutex); 
    }
    num_bytesP=Num_bytes;
    memcpy(rx_bufferP,Rx_buffer,Num_bytes);
    Mac_low_mutex_var=0;
    //msg("[BYPASS] RX_HANDLER SIGNAL MAC_LOW\n");
    pthread_cond_signal(&Mac_low_cond); //on ne peut que signaler depuis un context linux (rtf_handler); pas de wait, jamais!!!!!!
    pthread_mutex_unlock(&Mac_low_mutex);
  }
}
#endif //USER_MODE

/******************************************************************************************************/ 
void  bypass_signal_mac_phy(){
/******************************************************************************************************/ 
  char tt=1;   

  if(Master_list_rx != Master_list){
#ifndef USER_MODE
    rtf_put(fifo_mac_bypass,&tt,1);  // the Rx window is still opened  (Re)signal bypass_phy (emulate MAC signal)  
#endif //USER_MODE      
    bypass_rx_data();
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
void bypass_tx_data(char Type){
  /***************************************************************************/
  unsigned int         num_flows;
  bypass_msg_header_t *messg;
  unsigned int         byte_tx_count;
  messg = (bypass_msg_header_t *) (&bypass_tx_buffer[sizeof (bypass_proto2multicast_header_t)]);
  num_flows = 0;
  byte_tx_count = sizeof (bypass_msg_header_t) + sizeof (bypass_proto2multicast_header_t);
  if(Type==CHBCH_DATA){
    messg->Message_type = BYPASS_CHBCH_DATA;
    tx_handler(CHBCH_DATA,bypass_tx_buffer, &byte_tx_count, &num_flows);
    //    msg("[BYPASS] [TX_DATA] SEND  %d BYTES OF CH_DATA\n",byte_tx_count);
  }
  else if(Type==UL_DL_DATA){
    messg->Message_type = BYPASS_UL_DL_DATA;
    tx_handler(UL_DL_DATA,bypass_tx_buffer, &byte_tx_count, &num_flows);
    //msg("[BYPASS] [TX_DATA] SEND  %d BYTES OF UE_DATA\n",byte_tx_count);
  }
  else{
    messg->Message_type = BYPASS_MESSAGE_TYPE_CONTROL_BROADCAST;
    //msg("[BYPASS] [TX_DATA] SEND  %d BYTES OF MASTER_CONTROL\n",byte_tx_count);
  } 
  messg->M_id=Master_id;
  messg->Nb_flows = num_flows;
  ((bypass_proto2multicast_header_t *) bypass_tx_buffer)->size = byte_tx_count - sizeof (bypass_proto2multicast_header_t);
  //if(mac_xface->frame%1000==0)   
  multicast_link_write_sock (0, bypass_tx_buffer, byte_tx_count);
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

