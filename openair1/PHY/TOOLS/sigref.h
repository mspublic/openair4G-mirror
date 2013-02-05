#ifndef __PHY_TOOLS_SIGREF__H__
#define __PHY_TOOLS_SIGREF__H__

#include "PHY/defs.h"

// Helper functions to index slots and symbols in [tr]xdata/[tr]xdataF vectors
static inline int* get_symbol_ref_f(int** dataF, LTE_DL_FRAME_PARMS* frame_parms, u8 antenna, u8 slot, u8 symbol) {
#ifdef IFFT_FPGA
  const int symbol_size = 12*frame_parms->N_RB_DL;
#else // IFFT_FPGA
  const int symbol_size = frame_parms->ofdm_symbol_size;
#endif
  return &dataF[antenna][symbol_size*(slot*(frame_parms->symbols_per_tti>>1)+symbol)];
}

static inline int* get_ue_symbol_ref_f(PHY_VARS_UE* phy_vars_ue, int eNB_id, u8 antenna, u8 slot, u8 symbol) {
  return get_symbol_ref_f(phy_vars_ue->lte_ue_common_vars[eNB_id]->txdataF,
      phy_vars_ue->lte_frame_parms[eNB_id], antenna, slot, symbol);
}

static inline int* get_slot_ref_f(int** dataF, LTE_DL_FRAME_PARMS* frame_parms, u8 antenna, u8 slot) {
  return get_symbol_ref_f(dataF, frame_parms, antenna, slot, 0);
}

static inline int* get_ue_slot_ref_f(PHY_VARS_UE* phy_vars_ue, int eNB_id, u8 antenna, u8 slot) {
  return get_ue_symbol_ref_f(phy_vars_ue, eNB_id, antenna, slot, 0);
}

static inline int* get_slot_ref(int** data, LTE_DL_FRAME_PARMS* frame_parms, u8 antenna, u8 slot) {
  return &data[antenna][slot*(frame_parms->samples_per_tti>>1)];
}

static inline int* get_ue_slot_ref(PHY_VARS_UE* phy_vars_ue, int eNB_id, u8 antenna, u8 slot) {
  return get_slot_ref(phy_vars_ue->lte_ue_common_vars[eNB_id]->txdata,
      phy_vars_ue->lte_frame_parms[eNB_id], antenna, slot);
}

static inline int* get_symbol_ref(int** data, LTE_DL_FRAME_PARMS* frame_parms, u8 antenna, u8 slot, u8 symbol) {
  int* slot_data = get_slot_ref(data, frame_parms, antenna, slot);
  if(symbol == 0)
    return slot_data;
  else
    return &slot_data[frame_parms->nb_prefix_samples0 + frame_parms->ofdm_symbol_size +
      (symbol-1)*(frame_parms->nb_prefix_samples + frame_parms->ofdm_symbol_size)];
}

static inline int* get_ue_symbol_ref(PHY_VARS_UE* phy_vars_ue, int eNB_id, u8 antenna, u8 slot, u8 symbol) {
  return get_symbol_ref(phy_vars_ue->lte_ue_common_vars[eNB_id]->txdata,
      phy_vars_ue->lte_frame_parms[eNB_id], antenna, slot, symbol);
}

static inline int get_slot_length_f(LTE_DL_FRAME_PARMS* frame_parms) {
#ifdef IFFT_FPGA
  return frame_parms->N_RB_DL*12*(frame_parms->symbols_per_tti>>1);
#else // IFFT_FPGA
  return frame_parms->ofdm_symbol_size*(frame_parms->symbols_per_tti>>1);
#endif
}

static inline int get_symbol_length_f(LTE_DL_FRAME_PARMS* frame_parms) {
#ifdef IFFT_FPGA
  return frame_parms->N_RB_DL*12;
#else // IFFT_FPGA
  return frame_parms->ofdm_symbol_size;
#endif
}

#endif //__PHY_TOOLS_SIGREF__H__
