#include "oaisim.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "UTIL/FIFO/pad_list.h"

void get_simulation_options(int argc, char *argv[]);

void check_and_adjust_params();

void init_omv();

void init_seed(u8 set_seed);

void init_openair1();

void init_openair2();

void init_ocm();

void init_otg_pdcp_buffer();

void update_omg();

void update_omg_ocm();

void update_ocm();

void update_otg_eNB(int module_id, unsigned int ctime);

void update_otg_UE(int module_id, unsigned int ctime);

void exit_fun(const char* s);

void init_pad();

#ifdef XFORMS
void
do_forms (FD_phy_procedures_sim * form,
          LTE_UE_PDSCH ** lte_ue_pdsch_vars, LTE_eNB_PUSCH ** lte_eNB_pusch_vars, struct complex **ch, u32 ch_len);

void do_forms2(FD_lte_scope *form, LTE_DL_FRAME_PARMS *frame_parms,
               short ***channel,
               short **channel_f,
               short **rx_sig,
               short **rx_sig_f,
               short *dlsch_comp,
               short* dlsch_comp_i,
               short* dlsch_llr,
               short* pbch_comp,
               char *pbch_llr,
               int coded_bits_per_codeword,
               PHY_MEASUREMENTS *phy_meas);

void ia_receiver_on_off( FL_OBJECT *button, long arg);

void init_xforms();

void do_xforms();

#endif
