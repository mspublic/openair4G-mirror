#ifndef USER_MODE
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#endif
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "RRC/MESH/extern.h"
#include "LAYER2/PDCP/pdcp.h"

#ifndef USER_MODE
static int openair2_stats_read(char *buffer, char **my_buffer, off_t off, int length)
#else
int openair2_stats_read(char *buffer, char **my_buffer, off_t off, int length)
#endif
{

  int len = 0,fg,Overhead, Sign;
  unsigned int i,j,k,kk;
  unsigned int Mod_id = 0,CH_index;
  unsigned int tx_pdcp_sdu;
  unsigned int tx_pdcp_sdu_discarded;
  unsigned int tx_retransmit_pdu_unblock;
  unsigned int tx_retransmit_pdu_by_status;
  unsigned int tx_retransmit_pdu;
  unsigned int tx_data_pdu;
  unsigned int tx_control_pdu;
  unsigned int rx_sdu;
  unsigned int rx_error_pdu;  
  unsigned int rx_data_pdu;
  unsigned int rx_data_pdu_out_of_window;
  unsigned int rx_control_pdu;

  //    if (mac_xface->is_cluster_head == 0) {
  for (k=0;k<NB_INST;k++){



    if (Mac_rlc_xface->Is_cluster_head[k] == 0){
#ifndef PHY_EMUL_ONE_MACHINE
      Mod_id=k-NB_CH_INST; 

      len+=sprintf(&buffer[len],"UE TTI: %d\n",Mac_rlc_xface->frame);

      for (CH_index = 0;CH_index<NB_CNX_UE;CH_index++) {
	

	if (UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Active==1) {
	  len+=sprintf(&buffer[len],"CH %d: Wideband SINR %d dB---\n",
		       CH_index,UE_mac_inst[Mod_id].Def_meas[CH_index].Wideband_sinr);
	  len+=sprintf(&buffer[len],"CH %d: Subband SINR (dB) :",
		       CH_index);
	  for (fg=0;fg<NUMBER_OF_MEASUREMENT_SUBBANDS;fg++)
	    len+=sprintf(&buffer[len],"%d ",UE_mac_inst[Mod_id].Def_meas[CH_index].Sinr_meas[0][fg]);
	  len+=sprintf(&buffer[len],"\n");
	  
	  	
	  len+=sprintf(&buffer[len],"BCCH %d, NB_RX_MAC = %d (%d errors)\n",
		       UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.Lchan_id.Index,
		       UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.NB_RX,
		       UE_mac_inst[Mod_id].Bcch_lchan[CH_index].Lchan_info.NB_RX_ERRORS);
	  
	  
	  
	  len+=sprintf(&buffer[len],"CCCH %d, NB_RX_MAC = %d (%d errors)\n",
		       UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.Lchan_id.Index,
		       UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.NB_RX,
		     UE_mac_inst[Mod_id].Ccch_lchan[CH_index].Lchan_info.NB_RX_ERRORS);
	  
	  
	  len+=sprintf(&buffer[len],"LCHAN %d (DCCH), NB_TX_MAC = %d (%d bits/TTI, %d kbits/sec), NB_RX_MAC = %d (%d errors)\n",
		       UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.Lchan_id.Index,
		       UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.NB_TX,
		       UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.output_rate,
		       (10*UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.output_rate)>>5,
		       UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.NB_RX,
		       UE_mac_inst[Mod_id].Dcch_lchan[CH_index].Lchan_info.NB_RX_ERRORS);
	
	for(i=1;i<NB_RAB_MAX;i++){
	  if (UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Active==1) {
	    Overhead=UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.output_rate - Pdcp_stats_tx_rate[k][CH_index][i];
	    if(Overhead<0){
	      Overhead=-Overhead;
	      Sign=-1;
	    }
	    else Sign=1;
	      len+=sprintf(&buffer[len],"[PDCP]LCHAN %d: NB_TX = %d ,Tx_rate =(%d bits/TTI ,%d Kbits/s), NB_RX = %d ,Rx_rate =(%d bits/TTI ,%d Kbits/s) , LAYER2 TX OVERHEAD: %d Kbits/s\n",
			   UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.Lchan_id.Index,
			   Pdcp_stats_tx[k][CH_index][i],
			   Pdcp_stats_tx_rate[k][CH_index][i],
			   (10*Pdcp_stats_tx_rate[k][CH_index][i])>>5,
			   Pdcp_stats_rx[k][CH_index][i],
			   Pdcp_stats_rx_rate[k][CH_index][i],
			   (10*Pdcp_stats_rx_rate[k][CH_index][i])>>5,
			   Sign*(10*Overhead)>>5);


        int status =  rlc_stat_req     (k, 
                                              UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.Lchan_id.Index,
							  &tx_pdcp_sdu,
							  &tx_pdcp_sdu_discarded,
							  &tx_retransmit_pdu_unblock,
							  &tx_retransmit_pdu_by_status,
							  &tx_retransmit_pdu,
							  &tx_data_pdu,
							  &tx_control_pdu,
							  &rx_sdu,
							  &rx_error_pdu,  
							  &rx_data_pdu,
							  &rx_data_pdu_out_of_window,
							  &rx_control_pdu) ;
							  
		if (status == RLC_OP_STATUS_OK) {
	    len+=sprintf(&buffer[len],"RLC LCHAN %d, NB_SDU_TO_TX = %d\tNB_SDU_DISC %d\tNB_RX_SDU %d\n",
            UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.Lchan_id.Index, 
			   tx_pdcp_sdu,
			   tx_pdcp_sdu_discarded,
			   rx_sdu);
	    len+=sprintf(&buffer[len],"RLC LCHAN %d, NB_TB_TX_DATA = %d\tNB_TB_TX_CONTROL %d\tNB_TX_TB_RETRANS %d",
            UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.Lchan_id.Index, 
			   tx_data_pdu,
			   tx_control_pdu,
			   tx_retransmit_pdu);
	    len+=sprintf(&buffer[len],"\tRLC LCHAN %d, NB_TX_TB_RETRANS_BY_STATUS = %d\tNB_TX_TB_RETRANS_PADD %d\n",
            UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.Lchan_id.Index, 
			   tx_retransmit_pdu_by_status,
			   tx_retransmit_pdu_unblock);
	    len+=sprintf(&buffer[len],"RLC LCHAN %d, NB_RX_DATA = %d\tNB_RX_TB_OUT_WIN %d\tNB_RX_TB_CORRUPT %d\n",
            UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.Lchan_id.Index, 
			   rx_data_pdu,
			   rx_data_pdu_out_of_window,
			   rx_error_pdu);
		}

	    len+=sprintf(&buffer[len],"[MAC]: LCHAN %d, NB_TX_MAC = %d (%d bits/TTI, %d kbits/s), NB_RX_MAC = %d (%d errors)\n",
			   UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.Lchan_id.Index,
			   UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_TX,
			   UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.output_rate,
			   (10*UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.output_rate)>>5,
			   UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_RX,
			   UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_RX_ERRORS);
	      len+=sprintf(&buffer[len],"        TX per TB: ");
	      for(kk=0;kk<MAX_NUMBER_TB_PER_LCHAN/2;kk++)
		len+=sprintf(&buffer[len],"%d . ",UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_TX_TB[kk]);
	      len+=sprintf(&buffer[len],"\n");
	      len+=sprintf(&buffer[len],"        RXerr per TB: ");
	      for(kk=0;kk<MAX_NUMBER_TB_PER_LCHAN/2;kk++)
		len+=sprintf(&buffer[len],"%d/%d . ",UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_RX_ERRORS_TB[kk],
			     UE_mac_inst[Mod_id].Dtch_lchan[i][CH_index].Lchan_info.NB_RX_TB[kk]);
	      len+=sprintf(&buffer[len],"\n");



	    }

	  }
	}
      }
#endif //PHY_EMUL_ONE_MACHINE
    }

    else if(Mac_rlc_xface->Is_cluster_head[k] ==1){

      Mod_id=k; 
      len+=sprintf(&buffer[len],"-------------------------------------------------------------------CH %d: TTI: %d------------------------------------------------------------------\n",
		   NODE_ID[Mod_id],Mac_rlc_xface->frame);

      for(i=1;i<=NB_CNX_CH;i++){
	if (CH_mac_inst[Mod_id].Dcch_lchan[i].Active==1) {
	  len+=sprintf(&buffer[len],"\nMR index %d: DL SINR (feedback) %d dB, CQI: %s\n\n",
		       i,//CH_rrc_inst[Mod_id].Info.UE_list[i].L2_id[0],
		       CH_mac_inst[Mod_id].Def_meas[i].Wideband_sinr);
		       //print_cqi(CH_mac_inst[Mod_id].Def_meas[i].cqi));

	  len+=sprintf(&buffer[len],"[MAC] LCHAN %d (DCCH), NB_TX_MAC= %d (%d bits/TTI, %d kbits/s), NB_RX_MAC= %d (errors %d, sacch errors %d, sach errors %d, sach_missing %d)\n\n",
		       CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.Lchan_id.Index,
		       CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_TX,
		       CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.output_rate,
		       (10*CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.output_rate)>>5,
		       CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_RX,
		       CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_RX_ERRORS,
		       CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_RX_SACCH_ERRORS,
		       CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_RX_SACH_ERRORS,
		       CH_mac_inst[Mod_id].Dcch_lchan[i].Lchan_info.NB_RX_SACH_MISSING);
	
	  for(j=0;j<NB_RAB_MAX;j++){
	    if (CH_mac_inst[Mod_id].Dtch_lchan[j][i].Active==1){ 
	      Overhead=CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.output_rate - Pdcp_stats_tx_rate[k][i][j];
	      if(Overhead<0){
		Overhead=-Overhead;
		Sign=-1;
	      }
	      else Sign=1;
	      len+=sprintf(&buffer[len],"[PDCP]LCHAN %d: NB_TX = %d ,Tx_rate =(%d bits/TTI ,%d Kbits/s), NB_RX = %d ,Rx_rate =(%d bits/TTI ,%d Kbits/s), LAYER2 TX OVERHEAD= %d Kbits/s\n",
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index,
			   Pdcp_stats_tx[k][i][j],
			   Pdcp_stats_tx_rate[k][i][j],
			   (10*Pdcp_stats_tx_rate[k][i][j])>>5,
			   Pdcp_stats_rx[k][i][j],
			   Pdcp_stats_rx_rate[k][i][j],
			   (10*Pdcp_stats_rx_rate[k][i][j])>>5,
			   Sign*(10*Overhead)>>5);
	      int status =  rlc_stat_req     (k, 
                                              CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index,
							  &tx_pdcp_sdu,
							  &tx_pdcp_sdu_discarded,
							  &tx_retransmit_pdu_unblock,
							  &tx_retransmit_pdu_by_status,
							  &tx_retransmit_pdu,
							  &tx_data_pdu,
							  &tx_control_pdu,
							  &rx_sdu,
							  &rx_error_pdu,  
							  &rx_data_pdu,
							  &rx_data_pdu_out_of_window,
							  &rx_control_pdu) ;
		/*					  
		if (status == RLC_OP_STATUS_OK) {
	    len+=sprintf(&buffer[len],"\t[RLC] LCHAN %d, NB_SDU_TO_TX = %d\tNB_SDU_DISC %d\tNB_RX_SDU %d\n",
            CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index, 
			   tx_pdcp_sdu,
			   tx_pdcp_sdu_discarded,
			   rx_sdu);
	    len+=sprintf(&buffer[len],"\t[RLC] LCHAN %d, NB_TB_TX_DATA = %d\tNB_TB_TX_CONTROL %d\tNB_TX_TB_RETRANS %\n",
            CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index, 
			   tx_data_pdu,
			   tx_control_pdu,
			   tx_retransmit_pdu);
	    len+=sprintf(&buffer[len],"\t[RLC] LCHAN %d, NB_TX_TB_RETRANS_BY_STATUS = %d\tNB_TX_TB_RETRANS_PADD %d\n",
            CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index, 
			   tx_retransmit_pdu_by_status,
			   tx_retransmit_pdu_unblock);
	    len+=sprintf(&buffer[len],"\t[RLC] LCHAN %d, NB_RX_DATA = %d\tNB_RX_TB_OUT_WIN %d\tNB_RX_TB_CORRUPT %d\n",
            CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index, 
			   rx_data_pdu,
			   rx_data_pdu_out_of_window,
			   rx_error_pdu);
		}
		*/
	      len+=sprintf(&buffer[len],"[MAC]LCHAN %d (CNX %d,RAB %d), NB_TX_MAC= %d (%d bits/TTI, %d kbit/s), NB_RX_MAC= %d (errors %d, sacch_errors %d, sach_errors %d, sach_missing %d)\n",
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Lchan_id.Index,
			   i,j,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_TX,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.output_rate,
			   (10*CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.output_rate)>>5,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_RX,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_RX_ERRORS,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_RX_SACCH_ERRORS,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_RX_SACH_ERRORS,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_RX_SACH_MISSING);
	      len+=sprintf(&buffer[len],"[MAC][SCHEDULER] TX Arrival Rate %d, TX Service Rate %d, RX Arrival rate %d, RX Service rate %d, NB_BW_REQ_RX %d\n\n",
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Arrival_rate,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Tx_rate,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Req_rate,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.Rx_rate,
			   CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_BW_REQ_RX);

/*
	      len+=sprintf(&buffer[len],"        TX per TB: ");
	      for(kk=0;kk<MAX_NUMBER_TB_PER_LCHAN/2;kk++)
		len+=sprintf(&buffer[len],"%d.",CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_TX_TB[kk]);
	      len+=sprintf(&buffer[len],"\n");
	      len+=sprintf(&buffer[len],"        RXerr per TB: ");
	      for(kk=0;kk<MAX_NUMBER_TB_PER_LCHAN/2;kk++)
		len+=sprintf(&buffer[len],"%d/%d . ",CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_RX_ERRORS_TB[kk],
			     CH_mac_inst[Mod_id].Dtch_lchan[j][i].Lchan_info.NB_RX_TB[kk]);
	      len+=sprintf(&buffer[len],"\n");
*/
	    }
	  }
	}
      }
    
    }
  }
    return len;
}

#ifndef USER_MODE
static struct proc_dir_entry *proc_openair2_root;
/*
 * Initialize the module and add the /proc file.
 */
int add_openair2_stats()
{
  //#ifdef  KERNEL_VERSION_GREATER_THAN_2629
  struct proc_dir_entry *pde;
  //#endif

  proc_openair2_root = proc_mkdir("openair2",0);
  //#ifdef  KERNEL_VERSION_GREATER_THAN_2629
  // pde = proc_create_entry("lchan_stats", S_IFREG | S_IRUGO, proc_openair2_root);
  pde = create_proc_read_entry("lchan_stats", S_IFREG | S_IRUGO, proc_openair2_root, (read_proc_t*)&openair2_stats_read, NULL);
  if (!pde)
    printk("[OPENAIR][ERROR] can't create proc entry !\n");
  //#else  
  //create_proc_info_entry("lchan_stats", S_IFREG | S_IRUGO, proc_openair2_root, openair2_stats_read);
  //#endif 
 
  return 0;
}
/*
 * Unregister the file when the module is closed.
 */
void remove_openair2_stats()
{

  if (proc_openair2_root) {
    printk("[OPENAIR][CLEANUP] Removing openair proc entry\n");
    remove_proc_entry("lchan_stats", proc_openair2_root);
    //#ifdef  KERNEL_VERSION_GREATER_THAN_2629   
    
    //#else
    remove_proc_entry("openair2",NULL);
    //#endif;
  }
}
#endif
