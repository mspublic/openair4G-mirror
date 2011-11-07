/*
 * channel_sim_proc.h
 *
 */

#ifndef CHANNEL_SIM_PROC_H_
#define CHANNEL_SIM_PROC_H_


#endif /* CHANNEL_SIM_PROC_H_ */


void init_mmap_channel(int id,LTE_DL_FRAME_PARMS *frame_parms, double ***s_re,double ***s_im,double ***r_re,double ***r_im,double ***r_re0,double ***r_im0);

void init_mmap(int id,LTE_DL_FRAME_PARMS *frame_parms, double ***s_re,double ***s_im,double ***r_re,double ***r_im,double ***r_re0,double ***r_im0);

void Clean_Param(double **r_re,double **r_im,LTE_DL_FRAME_PARMS *frame_parms);

void do_DL_sig_channel_T(void *param);

void do_UL_sig_channel_T(void *param);

void init_rre(LTE_DL_FRAME_PARMS *frame_parms,double ***r_re0,double ***r_im0);

void do_DL_sig_eNB(double **r_re0,double **r_im0,double **r_re,double **r_im,double **s_re,double **s_im,
node_desc_t *enb_data[NUMBER_OF_eNB_MAX], u16 next_slot,LTE_DL_FRAME_PARMS *frame_parms);

void do_DL_sig_ue(double **r_re0,double **r_im0,double **r_re,double **r_im,double **s_re,double **s_im,u16 next_slot,LTE_DL_FRAME_PARMS *frame_parms);

void Channel_Add(int eNB_id,int UE_id,double **r_re,double **r_im,double **r_re0,double **r_im0,LTE_DL_FRAME_PARMS *frame_parms);

void do_UL_sig_ue(double **r_re0,double **r_im0,double **r_re,double **r_im,double **s_re,double **s_im,u16 next_slot,LTE_DL_FRAME_PARMS *frame_parms);

void do_UL_sig_eNB(double **r_re0,double **r_im0,double **r_re,double **r_im,double **s_re,double **s_im,u16 next_slot,LTE_DL_FRAME_PARMS *frame_parms);
