/*________________________openair_rrc_defs.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/


#ifndef __OPENAIR_RRC_DEFS_H__
#define __OPENAIR_RRC_DEFS_H__

#ifdef USER_MODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "PHY/defs.h"
#include "COMMON/platform_constants.h"

#include "COMMON/mac_rrc_primitives.h"
#include "LAYER2/MAC/defs.h"

//#include "COMMON/openair_defs.h"
#ifndef USER_MODE
#include <rtai.h>
#endif



#include "L3_rrc_defs.h"
#ifndef NO_RRM
#include "L3_rrc_interface.h"
#include "rrc_rrm_msg.h"
#include "rrc_rrm_interface.h"
#endif
#define NB_WAIT_CH_BCCH 4000000000 //max wait time(in frame) for CH BCCH 
#define NB_WAIT_MR_BCCH 20 //max wait time(in frame) for MR BCCH 

//#define NB_CH_CX 2  //max number of simulatneous connexions to CHs
//#define NB_UE_CX 16  //max number of simulatneous connexions to UEs

#define NB_TX_RACH_MAX 4000000000
//#define NUM_PRECONFIGURED_LCHAN (NB_CH_CX*2)  //BCCH, CCCH

#define RRC_WAIT_MEAS_CNT 150


#define NB_UE_BRDCAST 2


#define CH_READY 0
#define RRC_IDLE 1
#define RRC_PRE_SYNCHRO 2
#define RRC_PRE_ASSOCIATED 3
#define RRC_ASSOCIATED 4
#define RRC_CONNECTED 5


#define RRM_FREE(p)       if ( (p) != NULL) { free(p) ; p=NULL ; }
#define RRM_MALLOC(t,n)   (t *) malloc16( sizeof(t) * n ) 
#define RRM_CALLOC(t,n)   (t *) malloc16( sizeof(t) * n) 
#define RRM_CALLOC2(t,s)  (t *) malloc16( s ) 


#define MSG_L2ID(p) msg("[INFO] L2ID=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",\
	                (p).L2_id[0],(p).L2_id[1], (p).L2_id[2],(p).L2_id[3],\
	                (p).L2_id[4],(p).L2_id[5], (p).L2_id[6],(p).L2_id[7] )


#define PAYLOAD_SIZE_MAX 1024

#define NB_RAB_BRODCAST_MAX 2
#define MAX_ALLOWED_BCCH_MISS 10

typedef struct{
unsigned char  Status;
 unsigned int Next_check_frame;
}DTCH_BD_CONFIG;

typedef struct{
unsigned char Status;
unsigned char CH_id;
unsigned short UE_index;
unsigned int Rach_tx_cnt;
unsigned int Nb_bcch_wait;
unsigned int Nb_bcch_miss;
unsigned char Nb_rach_res; 
unsigned char Rach_time_alloc;
unsigned short Rach_freq_alloc;
 L2_ID CH_mac_id;
}UE_RRC_INFO;

typedef struct{
unsigned char Status;
unsigned char Nb_ue;
//unsigned short UE_index_list[NB_CNX_CH];
L2_ID UE_list[NB_CNX_CH];
DTCH_BD_CONFIG Dtch_bd_config[NB_CNX_CH];
}__attribute__ ((__packed__)) CH_RRC_INFO;

typedef struct{
  int Status;
union{
	UE_RRC_INFO UE_info;
	CH_RRC_INFO CH_info;
 }Info;
}RRC_INFO;




#define RRC_HEADER_SIZE_MAX 64
#define RRC_BUFFER_SIZE_MAX 1024
typedef struct{
  char Payload[RRC_BUFFER_SIZE_MAX];
  char Header[RRC_HEADER_SIZE_MAX];  
  unsigned short R_idx;
  unsigned short W_idx;
  unsigned short Nb_tb_max;
  unsigned short Tb_size;
  //LCHAN_DESC *Lchan_desc[2];
  void (*generate_fun)(u8);
  void (*decode_fun)(u8);
}RRC_BUFFER; 
#define RRC_BUFFER_SIZE sizeof(RRC_BUFFER)

typedef struct{
  u16 Rb_id;  //=Lchan_id
  // u8 UE_id;
  //u8 Lchan_type;
  LCHAN_DESC Lchan_desc[2];
   MAC_MEAS_REQ_ENTRY *Meas_entry;
  //u32 Next_check_frame
  //u16 Backlog_size;//from rlc
}RB_INFO;

typedef struct SRB_INFO{
  u16 Srb_id;  //=Lchan_id
  //  u8 UE_id;
  unsigned char Header_rx;
  unsigned char Header_tx;
  RRC_BUFFER Rx_buffer; 
  RRC_BUFFER Tx_buffer; 
  LCHAN_DESC Lchan_desc[2];
  unsigned char IP_addr_type;
  unsigned char IP_addr[16];
  unsigned char IP_addr_ok;
  unsigned int Trans_id;
  MAC_MEAS_REQ_ENTRY *Meas_entry;
  unsigned char Active;
  //u32 Next_check_frame; 
 //u16 Backlog_size;//from rlc
}SRB_INFO;

typedef struct{
  RB_INFO Rb_info;
  u8 Active;
  u32 Next_check_frame;
  u8 Status;
}RB_INFO_TABLE_ENTRY;

typedef struct{
  SRB_INFO Srb_info;
  u8 Active;
  u8 Status;
u32 Next_check_frame;
}SRB_INFO_TABLE_ENTRY;


typedef struct{
  //  unsigned char Rv_tb_idx;//4 MSB bits for Nb Tb; 4 LSB bits for Tb index
  //  unsigned char Tb_data_size;
  unsigned char CH_id;
  unsigned short UE_list[NB_UE_BRDCAST];
  //  unsigned char Nb_rach_res;
  //  unsigned char Rach_time_alloc;
  //  unsigned short Rach_freq_alloc;
}__attribute__ ((__packed__)) CH_BCCH_HEADER; 
#define CH_BCCH_HEADER_SIZE sizeof(CH_BCCH_HEADER)

typedef struct{
  unsigned char CH_id;
  unsigned short Rv_tb_idx;//4 MSB bits for Nb Tb; 4 LSB bits for Tb index
  unsigned short Tb_data_size;
  unsigned char Nb_cfg_req;
  unsigned char Nb_def_req;
  unsigned char Nb_meas_req;
  unsigned char Nb_sens_req;
}__attribute__ ((__packed__)) CH_CCCH_HEADER;
#define CH_CCCH_HEADER_SIZE sizeof(CH_CCCH_HEADER)

typedef struct{
  //unsigned char Rv_tb_idx;//4 MSB bits for Nb Tb; 4 LSB bits for Tb index
unsigned short Tb_data_size;
unsigned char Nb_meas_resp;
unsigned char Nb_def_resp;
}__attribute__ ((__packed__)) UE_DCCH_HEADER;
#define UE_DCCH_HEADER_SIZE sizeof(UE_DCCH_HEADER)

typedef struct{
u16 UE_id;
unsigned char Rach_time_alloc;
unsigned short Rach_freq_alloc;
}__attribute__ ((__packed__)) UE_CCCH_HEADER;
#define UE_CCCH_HEADER_SIZE sizeof(UE_CCCH_HEADER)

//#define NB_SRB_MAX 20
//#define NB_RB_MAX 100


typedef struct{
  u16 Node_id;
  CH_RRC_INFO Info;
  RB_INFO_TABLE_ENTRY Rab[NB_RAB_MAX][NB_CNX_CH+1];
  MAC_MEAS_REQ_ENTRY  Rab_meas[NB_RAB_MAX][NB_CNX_CH+1];
  RB_INFO_TABLE_ENTRY  Rab_dil[NB_RAB_MAX][NB_CNX_CH+1][NB_CNX_CH-1];
  MAC_MEAS_REQ_ENTRY  Rab_dil_meas[NB_RAB_MAX][NB_CNX_CH+1][NB_CNX_CH-1];
  SRB_INFO Srb0;
  SRB_INFO Srb1;
   SRB_INFO_TABLE_ENTRY Srb2[NB_CNX_CH+1];
   MAC_MEAS_REQ_ENTRY  Srb2_meas[NB_CNX_CH+1];
   DEFAULT_CH_MEAS *Def_meas[NB_CNX_CH+1];	
   char Rrc_dummy_pdu[TB_SIZE_MAX];
   u8 Nb_rb[NB_CNX_CH];
  //u8 Nb_rb_dil[NB_SIG_CNX_UE][NB_CNX_CH-1];
   unsigned char IP_addr_type;
   unsigned char IP_addr[16];
   L2_ID Mac_id;
   #ifndef NO_RRM
   rrm_init_scan_req_t Rrm_init_scan_req;
   #endif
   unsigned char Last_scan_req;

}CH_RRC_INST;


typedef struct{
  u16 Node_id;
  UE_RRC_INFO Info[NB_SIG_CNX_UE];
  RB_INFO_TABLE_ENTRY  Rab[NB_RAB_MAX][NB_CNX_UE];
  RB_INFO_TABLE_ENTRY  Rab_dil[NB_RAB_MAX][NB_SIG_CNX_UE][NB_CNX_CH-1];
  SRB_INFO Srb0[NB_SIG_CNX_UE];
  SRB_INFO Srb1[NB_SIG_CNX_UE];
  SRB_INFO_TABLE_ENTRY Srb2[NB_CNX_UE];
  DEFAULT_UE_MEAS *Def_meas[NB_CNX_UE];
  char Rrc_dummy_pdu[TB_SIZE_MAX];
  u8 Nb_rb[NB_SIG_CNX_UE];
  u8 Nb_rb_dil[NB_SIG_CNX_UE][NB_CNX_CH-1];
  //unsigned char IP_addr_type;
  //unsigned char IP_addr[16];
  L2_ID Mac_id;
}UE_RRC_INST;

//main.c
int rrc_init_global_param(void);
int L3_xface_init(void);
void openair_rrc_top_init(void);
char openair_rrc_ch_init(u8 Mod_id);
char openair_rrc_mr_init(u8 Mod_id,u8 CH_IDX);
void rrc_config_buffer(SRB_INFO *srb_info, u8 Lchan_type, u8 Role);
void openair_rrc_on(u8 Mod_id);
void rrc_switch_node_function(u8 Mod_id);
void ue_rrc_rx_tx(u8 Mod_id);
void ch_rrc_rx_tx(u8 Mod_id);
void ch_rrc_generate_bcch_header(u8 Mod_id);
void ue_rrc_decode_bcch(u8 Mod_id);
void ue_rrc_decode_bcch_header(u8 Mod_id , u8 Idx);
void ue_rrc_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info,u8);
void rrc_mac_association_req_tx(u8 Mod_id, unsigned char Idx);
unsigned char rrc_read_ccch_config_req(u8 Mod_id, SRB_INFO *Srb_info,u8);
void ch_rrc_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info);
void ch_rrc_decode_dcch(u8 , char*);
void ue_rrc_decode_dcch(u8 , char*,u8);
void ch_rrc_generate_bcch(u8 Mod_id);
char ch_rrc_generate_ccch(u8 Mod_id);
void mac_rrc_radio_meas_resp(MAC_MEAS_T *Mac_meas, MAC_MEAS_REQ_ENTRY * Meas_req_table_entry);
void  rrc_process_radio_meas(u8 Mod_id,MAC_MEAS_IND Mac_meas_ind,MAC_MEAS_REQ_ENTRY * Meas_entry);
void ch_disconnect_ue(unsigned char Mod_id,unsigned char UE_index);

//L2_interface.c
unsigned short rrc_fill_buffer(RRC_BUFFER *Rx_buffer, char *Data, unsigned short Size);
unsigned char mac_rrc_mesh_data_req( unsigned char Mod_id, unsigned short Srb_id, unsigned char Nb_tb,char *Buffer,u8);
unsigned char mac_rrc_mesh_data_ind( unsigned char Mod_id,  unsigned short Srb_id, char *Sdu, unsigned char Mui);
void mac_mesh_sync_ind( unsigned char Mod_id, unsigned char status);
void mac_rrc_mesh_meas_ind(u8,MAC_MEAS_REQ_ENTRY*);
void rlcrrc_mesh_data_ind( unsigned char Mod_id, u32 Rb_id, u32 sdu_size,char *Buffer);
void rrc_mesh_out_of_sync_ind(unsigned char Mod_id, unsigned short CH_index);
void def_meas_ind(u8 Mod_id,u8 CH_index);
//utils.c
u16 find_free_dtch_position(u8 Mod_id, u16 UE_CH_index);
u8 find_rrc_info_index(u8 Mod_id,u8 CH_id);
RB_INFO* rrc_find_rb_info(u8 Mod_id,u16 Rb_id);
u8 rrc_is_mobile_already_associated(u8 Mod_id, L2_ID Id);
u8 rrc_ue_is_mobilde_already_associated();
void rrc_reset_buffer(RRC_BUFFER *Rrc_buffer);
u8 rrc_is_node_isolated(u8 Mod_id);
SRB_INFO* rrc_find_srb_info(u8 Mod_id,u16 Srb_id);
#ifndef USER_MODE
char bcmp(void *x, void *y,int Size );
#endif

#ifndef NO_RRM
//rrc_config.c
 void rrc_init_ch_req(unsigned char Mod_id, rrm_init_ch_req_t  *msg);
void rrc_init_mr_req(unsigned char Mod_id, rrci_init_mr_req_t  *msg);
 void rrc_config_req(unsigned char Mod_id, void *msg, unsigned char Action,unsigned int Trans_id);



// rrm_msg.c
 void  fn_rrc (void);


 //rrc_msg
msg_t *msg_rrc_rb_meas_ind(unsigned char inst, RB_ID Rb_id, L2_ID L2_id, MEAS_MODE Meas_mode, MAC_RLC_MEAS_T *Mac_rlc_meas_t, unsigned int Trans_id );
msg_t *msg_rrc_sensing_meas_ind( unsigned char inst, L2_ID L2_id, unsigned int NB_meas, SENSING_MEAS_T *Sensing_meas, unsigned int Trans_id );
msg_t *msg_rrc_sensing_meas_resp( unsigned char inst, unsigned int Trans_id )	;
msg_t *msg_rrc_cx_establish_ind( unsigned char inst, L2_ID L2_id, unsigned int Trans_id,unsigned char *L3_info, L3_INFO_T L3_info_t,
									RB_ID DTCH_B_id, RB_ID DTCH_id );
msg_t *msg_rrc_phy_synch_to_MR_ind( unsigned char inst, L2_ID L2_id);
msg_t *msg_rrc_phy_synch_to_CH_ind( unsigned char inst, unsigned int Ch_index,L2_ID L2_id );
msg_t *msg_rrc_rb_establish_resp( unsigned char inst, unsigned int Trans_id  );
msg_t *msg_rrc_rb_establish_cfm( unsigned char inst, RB_ID Rb_id, RB_TYPE RB_type, unsigned int Trans_id );
msg_t *msg_rrc_rb_modify_resp( unsigned char inst, unsigned int Trans_id );
msg_t *msg_rrc_rb_modify_cfm(unsigned char inst, RB_ID Rb_id, unsigned int Trans_id  );
msg_t *msg_rrc_rb_release_resp( unsigned char inst, unsigned int Trans_id );
msg_t *msg_rrc_MR_attach_ind( unsigned char inst, L2_ID L2_id );
msg_t * msg_rrc_init_scan_req(Instance_t inst, L2_ID L2_id, unsigned int Interv);
			      
#endif
#endif
