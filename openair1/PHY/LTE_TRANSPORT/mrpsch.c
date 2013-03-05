/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2012 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/mrpsch.c
* \brief Top-level routines for generating mesh router synchronization signal (MRPSCH)
* \author A. Blad
* \date 2012
* \version 0.1
* \company Eurecom
* \email: anton.blad@liu.se
* \note
* \warning
*/

#define DEBUG_MRPSCH

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/TOOLS/sigref.h"

#include "mrpsch_tab.c"

/* The MRPSCH sync is done by computing the metric
 * M[n] = C[n] / E[n], where
 * C[n] = sum_{k=0}^{K-1} corr*[k] x[n+k] is the correlation of the received
 * signal with the time domain MRPSCH over the length of an OFDM symbol, and
 * E[n] = 1/K * sum_{k=0}^{K-1} x*[n+k] x[n+k] is the average energy of the
 * samples in the correlated received sequence. The MRPSCH signal is
 * considered to be found when 
 * E[n] >= MRPSCH_ENERGY_THRESHOLD, and
 * M[n] >= MRPSCH_CORR_THRESHOLD.
 */
const unsigned int MRPSCH_CORR_THRESHOLD = 100;
const unsigned int MRPSCH_ENERGY_THRESHOLD = 1000;

static int* mrpsch_corr[3] = {NULL, NULL, NULL};
static int* mrpsch_avg_energy[3] = {NULL, NULL, NULL};
static int* mrpsch_time = NULL;

int mrpsch_sync_init(LTE_DL_FRAME_PARMS *frame_parms)
{
  int k;
  int sector;
  int* sync_tmpF;
  int* sync_tmp;
  short* sym;

  // Allocate correlation vectors
  for(sector = 0; sector < 3; sector++) {
    mrpsch_corr[sector] = (int*) malloc16(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti*sizeof(int));
    if(mrpsch_corr[sector]) {
#ifdef DEBUG_MRPSCH
      LOG_D(PHY, "[MRPSCH]mrpsch_corr[%d] allocated at %p\n", sector, mrpsch_corr[sector]);
#endif
    }
    else {
      LOG_E(PHY, "[MRPSCH]mrpsch_corr[%d] not allocated\n", sector);
      return(-1);
    }
  }

  // Allocate normalization vector
  for(sector = 0; sector < 3; sector++) {
    mrpsch_avg_energy[sector] = (int*) malloc16(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti*sizeof(int));
    if(mrpsch_avg_energy[sector]) {
#ifdef DEBUG_MRPSCH
      LOG_D(PHY, "[MRPSCH]mrpsch_avg_energy[%d] allocated at %p\n", sector, mrpsch_avg_energy[sector]);
#endif
    }
    else {
      LOG_E(PHY, "[MRPSCH]mrpsch_avg_energy[%d] not allocated\n", sector);
      return(-1);
    }
  }

  // Allocate time domain signal vector
  mrpsch_time = (int*) malloc16(frame_parms->ofdm_symbol_size*sizeof(int));
  if(mrpsch_time) {
    bzero(mrpsch_time, frame_parms->ofdm_symbol_size*sizeof(int));
#ifdef DEBUG_MRPSCH
    LOG_D(PHY, "[MRPSCH]mrpsch_time allocated at %p\n", mrpsch_time);
#endif
  }
  else {
    LOG_E(PHY, "[MRPSCH]mrpsch_time not allocated\n");
    return(-1);
  }

  sync_tmpF = (int*) malloc16(frame_parms->ofdm_symbol_size*sizeof(int));
  if(sync_tmpF) {
    bzero(sync_tmpF, frame_parms->ofdm_symbol_size*sizeof(int));
  }
  else {
    LOG_E(PHY, "[MRPSCH]sync_tmpF not allocated\n");
    return(-1);
  }

  sync_tmp = (int*) malloc16(frame_parms->ofdm_symbol_size*2*sizeof(int));
  if(sync_tmp) {
    bzero(sync_tmp, frame_parms->ofdm_symbol_size*2*sizeof(int));
  }
  else {
    LOG_E(PHY, "[MRPSCH]sync_tmp not allocated\n");
    return(-1);
  }

  // Generate oversampled time domain sequence
  // (scaling to avoid overflow in fft)
  for(k = 0; k < 36; k++) {
    sym = (short*) &sync_tmpF[frame_parms->ofdm_symbol_size-3*12+k];
    sym[0] = mrpsch_real[k] >> 2;
    sym[1] = mrpsch_imag[k] >> 2;
  }
  for(k = 0; k < 36; k++) {
    sym = (short*) &sync_tmpF[1+k];
    sym[0] = mrpsch_real[36+k] >> 2;
    sym[1] = mrpsch_imag[36+k] >> 2;
  }

  fft((short*)sync_tmpF,         /// complex input
      (short*)sync_tmp,          /// complex output
      frame_parms->twiddle_ifft,    /// complex twiddle factors
      frame_parms->rev,             /// bit reversed permutation vector
      frame_parms->log2_symbol_size,/// log2(FFT_SIZE)
      (frame_parms->log2_symbol_size/2),
      0);                            /// 0 - input is in complex Q1.15 format, 1 - input is in complex redundant Q1.15 format)

  for(k = 0; k < frame_parms->ofdm_symbol_size; k++)
    mrpsch_time[k] = sync_tmp[k << 1];

  free(sync_tmpF);
  free(sync_tmp);

#ifdef USER_MODE
#ifdef DEBUG_MRPSCH
  write_output("mrpsch_time_v.m", "mrpsch_time", mrpsch_time, frame_parms->ofdm_symbol_size, 1, 1);
#endif
#endif

  return 1;
}

void mrpsch_sync_free(void)
{
  int sector;

#ifdef USER_MODE
  for(sector = 0; sector < 3; sector++) {
    if(mrpsch_corr[sector]) {
      free(mrpsch_corr[sector]);
    }
  }
  if(mrpsch_time) {
    free(mrpsch_time);
  }
#endif
}

#ifdef IFFT_FPGA

int generate_mrpsch(PHY_VARS_UE* phy_vars_ue, int eNB_id, short amp, unsigned short slot, unsigned short symbol)
{
  LTE_DL_FRAME_PARMS* frame_parms = &phy_vars_ue->lte_frame_parms[eNB_id];
  unsigned short k;
  const unsigned short antenna = 0;
  mod_sym_t* symbol_data;

  symbol_data = get_ue_symbol_ref_f(phy_vars_ue, eNB_id, antenna, slot, symbol);

  // Map the first 36 values to the last 3 RBs
  for(k = 0; k < 36; k++)
    symbol_data[(frame_parms->N_RB_DL-3)*12+k] = mrpsch_tab[k];

  // Map the last 36 values to the first 3 RBs
  for(k = 0; k < 36; k++)
    symbol_data[k] = mrpsch_tab[36+k];

  return 0;
}

#else // IFFT_FPGA

int generate_mrpsch(PHY_VARS_UE* phy_vars_ue, int eNB_id, short amp, unsigned short slot, unsigned short symbol)
{
  LTE_DL_FRAME_PARMS* frame_parms = phy_vars_ue->lte_frame_parms[eNB_id];
  unsigned short k;
  const unsigned short antenna = 0;
  mod_sym_t* symbol_data;
  short* sym;

  symbol_data = get_ue_symbol_ref_f(phy_vars_ue, eNB_id, antenna, slot, symbol);

  // Map the last 36 subcarriers
  for(k = 0; k < 36; k++) {
    sym = (short*) &symbol_data[frame_parms->ofdm_symbol_size - 3*12 + k];
    sym[0] = ((int)amp*mrpsch_real[k]) >> 15;
    sym[1] = ((int)amp*mrpsch_imag[k]) >> 15;
  }

  // Map the first 36 subcarriers (skip DC)
  for(k = 0; k < 36; k++) {
    sym = (short*) &symbol_data[1+k];
    sym[0] = ((int)amp*mrpsch_real[36+k]) >> 15;
    sym[1] = ((int)amp*mrpsch_imag[36+k]) >> 15;
  }

  return 0;
}
#endif // IFFT_FPGA


inline int norm32(int x)
{
  short* sym = (short*) &x;
  int x_r = sym[0];
  int x_i = sym[1];

  return x_r*x_r + x_i*x_i;
}

void mrpsch_correlate(LTE_DL_FRAME_PARMS* frame_parms, int*** rxdata,
    int first_sample, int last_sample, int* mrpsch_pos, int* mrpsch_source, int dec)
{
  unsigned int sync_pos;
  unsigned int sync_val;
  unsigned int n;
  unsigned int antenna;
  int sync_source;
  int result;
  int sector;
  short* sym;

#ifdef DEBUG_MRPSCH
  LOG_D(PHY, "MRPSCH: correlate: [%d,%d], dec=%d\n", first_sample, last_sample, dec);
#endif

  sync_pos = -1;
  sync_source = 0;
  sync_val = 0;

  // Compute average energy of input signal
  for(sector = 0; sector < 3; sector++) {
    mrpsch_avg_energy[sector][first_sample] = 0;
    for(n = first_sample; n < first_sample + frame_parms->ofdm_symbol_size; n += dec) {
      for(antenna = 0; antenna < frame_parms->nb_antennas_rx; antenna++) {
	mrpsch_avg_energy[sector][first_sample] += dec*norm32(rxdata[sector][antenna][n]) >> (frame_parms->log2_symbol_size);
      }
    }
    for(n = first_sample + dec; n < last_sample; n += dec) {
      mrpsch_avg_energy[sector][n] = mrpsch_avg_energy[sector][n-dec];
      for(antenna = 0; antenna < frame_parms->nb_antennas_rx; antenna++) {
	mrpsch_avg_energy[sector][n] += dec*norm32(rxdata[sector][antenna][frame_parms->ofdm_symbol_size+n-dec]) >> (frame_parms->log2_symbol_size);
	mrpsch_avg_energy[sector][n] -= dec*norm32(rxdata[sector][antenna][n-dec]) >> (frame_parms->log2_symbol_size);
      }
    }
  }

  for(n = first_sample; n < last_sample; n += dec) {
#ifdef RTAI_ENABLED
    // This is necessary since the sync takes a long time and it seems to block all other threads thus screwing up RTAI. If we pause it for a little while during its execution we give RTAI a chance to catch up with its other tasks.
    if ((n%frame_parms->samples_per_tti == 0) && (n>0) && (openair_daq_vars.sync_state==0)) {
#ifdef DEBUG_MRPSCH
      LOG_D(PHY, "[eNB%d][MRPSCH]Sync: pausing for 1000ns, n=%d\n", phy_vars_enb->Mod_id, n);
#endif
      rt_sleep(nano2count(1000));
    }
#endif

    for(sector = 0; sector < 3; sector++) {
      mrpsch_corr[sector][n] = 0;
    }

    // Correlate with time domain sequence for all sectors
    for(sector = 0; sector < 3; sector++) {
      sym = (short*)&mrpsch_corr[sector][n];
      sym[0] = 0;
      sym[1] = 0;
      for(antenna = 0; antenna < frame_parms->nb_antennas_rx; antenna++) {
	result = dot_product((short*)mrpsch_time, (short*) &(rxdata[sector][antenna][n]), frame_parms->ofdm_symbol_size, 15);

	sym[0] += ((short*) &result)[0];
	sym[1] += ((short*) &result)[1];
      }
    }

    // Calculate the absolute value of the correlation
    for(sector = 0; sector < 3; sector++) {
      mrpsch_corr[sector][n] = norm32(mrpsch_corr[sector][n]);
    }

    for(sector = 0; sector < 3; sector++) {
      if(mrpsch_avg_energy[sector][n] > MRPSCH_ENERGY_THRESHOLD) {
	if(mrpsch_corr[sector][n] > MRPSCH_CORR_THRESHOLD * mrpsch_avg_energy[sector][n]) {
	  if(mrpsch_corr[sector][n] > sync_val) {
	    sync_val = mrpsch_corr[sector][n];
	    sync_pos = n;
	    sync_source = sector;
	  }
	}
      }
    }
  }

#ifdef DEBUG_MRPSCH
#ifdef USER_MODE
  write_output("mrpsch_avg_energy_0_v.m", "mrpsch_avg_energy_0", mrpsch_avg_energy[0], FRAME_LENGTH_COMPLEX_SAMPLES, 1, 2);
  write_output("mrpsch_avg_energy_1_v.m", "mrpsch_avg_energy_1", mrpsch_avg_energy[1], FRAME_LENGTH_COMPLEX_SAMPLES, 1, 2);
  write_output("mrpsch_avg_energy_2_v.m", "mrpsch_avg_energy_2", mrpsch_avg_energy[2], FRAME_LENGTH_COMPLEX_SAMPLES, 1, 2);
  write_output("mrpsch_corr_0_v.m", "mrpsch_corr_0", mrpsch_corr[0], FRAME_LENGTH_COMPLEX_SAMPLES, 1, 2);
  write_output("mrpsch_corr_1_v.m", "mrpsch_corr_1", mrpsch_corr[1], FRAME_LENGTH_COMPLEX_SAMPLES, 1, 2);
  write_output("mrpsch_corr_2_v.m", "mrpsch_corr_2", mrpsch_corr[2], FRAME_LENGTH_COMPLEX_SAMPLES, 1, 2);
#endif
#endif

  if(sync_pos >= 0)
  {
    *mrpsch_pos = sync_pos;
    *mrpsch_source = sync_source;
  }
  else
  {
    *mrpsch_pos = -1;
  }
}

int mrpsch_sync(PHY_VARS_eNB* phy_vars_enb)
{
  int sync_pos;
  int sync_source;
  unsigned int symbol_pos;
  unsigned int expected_symbol_pos;
  LTE_DL_FRAME_PARMS* frame_parms = &phy_vars_enb->lte_frame_parms;

  mrpsch_correlate(frame_parms, phy_vars_enb->lte_eNB_common_vars.rxdata,
      0, LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti - frame_parms->ofdm_symbol_size,
      &sync_pos, &sync_source, 4);

  // Return -1 if MRPSCH not found
  if(sync_pos == -1) {
#ifdef DEBUG_MRPSCH
    LOG_D(PHY, "[eNB%d]MRPSCH: Not detected\n", phy_vars_enb->Mod_id);
#endif
    return -1;
  }

  // Compute symbol position
  symbol_pos = sync_pos - frame_parms->nb_prefix_samples;

  // Compute expected symbol position
  expected_symbol_pos = 4*(frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - frame_parms->nb_prefix_samples;

  // Compute receiver offset
  if(symbol_pos >= expected_symbol_pos)
    phy_vars_enb->rx_offset = symbol_pos - expected_symbol_pos;
  else
    phy_vars_enb->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + symbol_pos - expected_symbol_pos;

  phy_vars_enb->nb_lost_mrpsch = 0;

#ifdef DEBUG_MRPSCH
  LOG_D(PHY, "[eNB%d]MRPSCH: Sync estimated from sector %d, position %d, offset %d, corr %d/%d\n",
      phy_vars_enb->Mod_id, sync_source, sync_pos, phy_vars_enb->rx_offset, 
      mrpsch_corr[sync_source][sync_pos], mrpsch_avg_energy[sync_source][sync_pos]);
#endif

  return sync_pos;
}

int mrpsch_update_sync(PHY_VARS_eNB* phy_vars_enb, int search_range)
{
  int sync_pos;
  int sync_source;
  unsigned int expected_symbol_pos;
  unsigned int expected_sync_pos;
  LTE_DL_FRAME_PARMS* frame_parms = &phy_vars_enb->lte_frame_parms;

  // Compute expected sync position
  expected_symbol_pos = 4*(frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - frame_parms->nb_prefix_samples;
  expected_sync_pos = expected_symbol_pos + frame_parms->nb_prefix_samples;

  mrpsch_correlate(frame_parms, phy_vars_enb->lte_eNB_common_vars.rxdata,
      expected_sync_pos - search_range, expected_sync_pos + search_range,
      &sync_pos, &sync_source, 4);

  // Return -1 if MRPSCH not found
  if(sync_pos == -1) {
    phy_vars_enb->nb_lost_mrpsch++;
#ifdef DEBUG_MRPSCH
    LOG_D(PHY, "[eNB%d]MRPSCH: not detected (nb_lost_mrpsch=%d)\n", phy_vars_enb->Mod_id, phy_vars_enb->nb_lost_mrpsch);
#endif
    if(phy_vars_enb->nb_lost_mrpsch == 1000) {
      phy_vars_enb->rx_offset = 0;
      return -1;
    }
    else {
      return 0;
    }
  }

  // Update receiver offset
  phy_vars_enb->rx_offset += sync_pos - expected_sync_pos;

  if(phy_vars_enb->rx_offset < 0)
    phy_vars_enb->rx_offset += FRAME_LENGTH_COMPLEX_SAMPLES;

  if(phy_vars_enb->rx_offset >= FRAME_LENGTH_COMPLEX_SAMPLES)
    phy_vars_enb->rx_offset -= FRAME_LENGTH_COMPLEX_SAMPLES;

#ifdef DEBUG_MRPSCH
  LOG_D(PHY, "[eNB%d]MRPSCH: Sync updated from sector %d, position %d, offset %d, corr %d/%d\n",
      phy_vars_enb->Mod_id, sync_source, sync_pos, phy_vars_enb->rx_offset, 
      mrpsch_corr[sync_source][sync_pos], mrpsch_avg_energy[sync_source][sync_pos]);
#endif

  return sync_pos;
}

