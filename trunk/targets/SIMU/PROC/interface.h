
#ifndef __INTERFACE_H__
#    define __INTERFACE_H__


typedef struct {
      void*           pointer;
  } memory_reference_t2;



typedef struct {
         memory_reference_t2  mem_ref;
          }obj_ref_t2;

typedef struct{
  obj_ref_t2 *gm;
}HO;


typedef struct {
void*           pointer;
int port;
int Exec_Msg_Flag;
int node_id;
int node_role ;
int next_slot;
int last_slot;
int frame;
int sub_frame;
int slot;
int Exec_FLAG;
int EResp_FLAG;
int ue_id;
}s_t;

HO Instance[3];


typedef struct{
	int thread_id;
	int eNB_id;
	int UE_id;
	double **s_re;
	double **s_im;
	double **r_re;
	double **r_im;
	double **r_re0;
	double **r_im0;
	struct channel_desc_t *eNB2UE;
	struct channel_desc_t *UE2eNB;
	struct node_desc_t *enb_data;
	struct node_desc_t *ue_data;
	int *next_slot;
	int *abstraction_flag;
	struct LTE_DL_FRAME_PARMS *frame_parms;
}ch_thread;


#define CHANNEL_PORT 37800
#define eNB_PORT 38800
#define UE_PORT 39800
#define MAX_eNB 20
#define MAX_UE 20
#endif
