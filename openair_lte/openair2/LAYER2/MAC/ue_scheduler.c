/*________________________ue_scheduler.c________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/

#include "defs.h"
#include "extern.h"
/***********************************************************************/
void ue_mac_scheduler_rx(u8 Mod_id) {
/***********************************************************************/
  u8 i=0;
  
  for(i=0;i<NB_SIG_CNX_UE;i++) {
    ue_get_chbch(Mod_id,i);   
  }
}

/***********************************************************************/
void ue_mac_scheduler_tx(u8 Mod_id) {
/***********************************************************************/
  u8 i=0;


  // for(i=0;i<NB_SIG_CNX_UE;i++) 
  //    ue_scheduler(Mod_id,i);

 for(i=0;i<NB_SIG_CNX_UE;i++) 
    ue_generate_rach(Mod_id,i);  
  

  ue_generate_sach(Mod_id);   

  
 
}




/*
void macphy_update_measurement(unsigned char Mod_id){
  unsigned char i,j;
  for(i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++)
    for(j=0;j<NB_TIME_ALLOC;j++){
      if( Meas_info_matrix[Mod_id][i][j] > 0 )
	Meas_info_matrix[Mod_id][i][j].Rssi = (((Rssi_meas[Mod_id][i][j] - Meas_info_matrix[Mod_id][i][j].Rssi) * RSSI_FFUP )/ RSSI_FFDW ) 
	  + Meas_info_matrix[Mod_id][i][j];
      else 
	Meas_info_matrix[Mod_id][i][j].Rssi = Rssi_meas[Mod_id][i][j];
      Rssi_meas[Mod_id][i][j]=0;
    }
}


void scheduler_update_global_rssi_measurement(unsigned char Mod_id, unsigned char UE_index,GLOBAL_MEAS_IND *Global_meas_ind){
  unsigned char i,j;
  for(i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++)
    for(j=0;j<NB_TIME_ALLOC;j++){
      if( Global_meas_ind->Meas_report_frame > Sched_trace_matrix[i][j].Fisrt_sched_frame )
	for( k=0 ; k < Sched_trace_matrix[i][j].Nb_parallel_sched_user ; k++){
	  Sched_rssi_meas_matrix[Mod_id][UE_index][Sched_trace_matrix[i][j].Txmit_index[k]][i][j].Rssi=Global_meas_ind->Rssi[i][j];
      }
}
*/
