/* Authors: Florian Kaltenberger, Leonardo Cardoso */
   
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"

// Temporary buffer for building the avarage of pilots
// This is explicitly set to 32 bits to guarantee that
// the accumlulation for the average procedure doesnt
// overflow
static int temp_buffer[NB_ANTENNAS_RX][OFDM_SYMBOL_SIZE_SAMPLES_MAX] __attribute__ ((aligned(16)));
static short temp_shrt_buf[OFDM_SYMBOL_SIZE_SAMPLES_MAX] __attribute__ ((aligned(16))); // Output of the average operation
static short out_sch[OFDM_SYMBOL_SIZE_SAMPLES_MAX*2] __attribute__ ((aligned(16)));			// Ouput of the phase correction procedure
// (*2 since it is in MMX format)
static int nb_pilots_total;	// total number of pilots that were averaged


void phy_channel_est_emos(int ref_pilot, int from_pilot, int to_pilot, int sch_index, int perform_estimate, int ignore_prefix)
{
#ifdef USER_MODE
  char fname[40], vname[40];
#endif // USER_MODE

  int time_in,time_out;
  int aa, j, pilot_offset;
  short *rx_buffer;
  short *ref_buffer;


  /*
  if ((mac_xface->frame % 100) == 0)
    msg("[PHY][CHANNEL ESTIMATION][EMOS] Frame %d: ref_pilot %d, from_pilot %d, to_pilot %d, sch_index %d\n",
	mac_xface->frame, ref_pilot, from_pilot, to_pilot, sch_index);
  */

  //memset(temp_buffer, 0x0, OFDM_SYMBOL_SIZE_SAMPLES * sizeof(int));

  //msg("[PHY][CHANNEL ESTIMATION][EMOS] Starting antenna loop\n");
 
#ifndef USER_MODE 
  time_in = openair_get_mbox();
#endif // USER_MODE

  for (aa = 0; aa < NB_ANTENNAS_RX; aa++) {
    // Derotation and averaging
    // Gets the initial sch for the reference
    if (ignore_prefix == 1)
    {
      ref_buffer = (short *) &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(ref_pilot) << (LOG2_NUMBER_OF_OFDM_CARRIERS)];
    }
    else
    {
      ref_buffer = (short *) &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(ref_pilot << (LOG2_NUMBER_OF_OFDM_CARRIERS))
                   + (1 + from_pilot) * CYCLIC_PREFIX_LENGTH];
    }

#ifdef USER_MODE
#ifdef DEBUG_PHY
    sprintf (fname, "refsch_ant%d.m", aa);
    sprintf (vname, "refsch_ant%d",  aa);
		
    msg("ref_buffer = %d\n",ref_buffer);

    write_output (fname,
                  vname,
                  (short *)ref_buffer,
                  2 * NUMBER_OF_OFDM_CARRIERS,
                  2,
                  1);
#endif //DEBUG_PHY
#endif //USER_MODE

    //msg("[PHY][CHANNEL ESTIMATION][EMOS] Initialization of the 32 bit buffer\n");

    // Initialization of the 32 bit buffer
    if (ref_pilot==from_pilot)
      {
	for (j = 0; j < OFDM_SYMBOL_SIZE_SAMPLES; j++)
	  {
	    temp_buffer[aa][j] = (int) ref_buffer[j];
	  }
      }
    
    for (pilot_offset = from_pilot + 1; pilot_offset <= to_pilot; pilot_offset++)
      {
	if (ignore_prefix == 1)
	  {
	    rx_buffer = (short *) &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[(pilot_offset) << (LOG2_NUMBER_OF_OFDM_CARRIERS)];
	  }
	else
	  {
	    rx_buffer = (short *) &PHY_vars->rx_vars[aa].RX_DMA_BUFFER[((pilot_offset) << (LOG2_NUMBER_OF_OFDM_CARRIERS))
								       + (pilot_offset) * CYCLIC_PREFIX_LENGTH];
	  }
 
#ifdef USER_MODE
#ifdef DEBUG_PHY
      // DEBUG CODE
      sprintf (fname, "rx_buffer%d_ant%d.m", pilot_offset, aa);
      sprintf (vname, "rx_buffer%d_ant%d", pilot_offset, aa);
			
      msg("rx_buffer = %d\n",rx_buffer);

      write_output (fname,
                    vname,
                    (short *)rx_buffer,
                    2 * NUMBER_OF_OFDM_CARRIERS,
                    2,
                    1);
      // END DEBUG CODE
#endif //DEBUG_PHY
#endif //USER_MODE


      
      phy_phase_compensation(ref_buffer, rx_buffer, out_sch, 0, aa, &(PHY_vars->sch_data[sch_index].perror[aa][pilot_offset-from_pilot]));
      
      // out_sch has format | Re0 Im0 Re0 Im0 || Re1 Im1 Re1 Im1 |
      // TODO: it would be efficient to keep this format for the fft routine

      for (j = 0; j < OFDM_SYMBOL_SIZE_SAMPLES; j+=2)
      {
        temp_buffer[aa][j] += (int) out_sch[j<<1];
        temp_buffer[aa][j+1] += (int) out_sch[(j<<1)+1];
      }

    } // pilot loop
  } //antenna loop

#ifndef USER_MODE
  time_out = openair_get_mbox();
#endif //USER_MODE

/*   if ((mac_xface->frame % 100) == 0) */
/*    	      msg("[PHY][CHANNEL ESTIMATION][EMOS] Frame %d: phy_phase_compensation: time in %d, time out %d\n", */
/*    		  mac_xface->frame, */
/* 		  time_in, */
/* 		  time_out); */

  if (ref_pilot==from_pilot)
    nb_pilots_total = 0;
  
  nb_pilots_total = nb_pilots_total + (to_pilot - from_pilot);

  if (perform_estimate==TRUE)
  {
#ifndef USER_MODE	  
    time_in = openair_get_mbox();
#endif //USER_MODE    
    for (aa = 0; aa < NB_ANTENNAS_RX; aa++)
    {
      for (j = 0; j < OFDM_SYMBOL_SIZE_SAMPLES; j++)
      {
        temp_shrt_buf[j] = (short)(temp_buffer[aa][j] / nb_pilots_total);
      }

      fft ((short *) temp_shrt_buf,
           (short *) &PHY_vars->sch_data[sch_index].rx_sig_f[aa][0],
           (short *) twiddle_fft,
           rev,
           LOG2_NUMBER_OF_OFDM_CARRIERS,
           3,
           0);


      phy_channel_estimation ((short *) &PHY_vars->sch_data[sch_index].rx_sig_f[aa][0],
                              (short *) &PHY_vars->sch_data[sch_index].channel[aa][0],
                              (short *) &PHY_vars->sch_data[sch_index].channel_f[aa][0],
			      (short *) &PHY_vars->sch_data[sch_index].channel_matched_filter_f[aa][0],
                              (short *) &PHY_vars->sch_data[sch_index].SCH_conj_f[0],
                              15, //LOG2_SCH_RX_F_AMP,
                              (NB_ANTENNAS_RX == 1) ? 1 : 0);

#ifdef USER_MODE
#ifdef DEBUG_PHY


      sprintf (fname, "sch_channelF%d.m", aa);
      sprintf (vname, "sch_chanF%d", aa);

      write_output (fname,
                    vname,
                    (short *) &PHY_vars->sch_data[sch_index].channel_f[aa][0],
                    2 * NUMBER_OF_OFDM_CARRIERS,
                    2,
                    1);


      sprintf (fname, "sch_channel%d.m", aa);
      sprintf (vname, "sch_chan%d", aa);

      write_output (fname,
                    vname,
                    (short *) &PHY_vars->sch_data[sch_index].channel[aa][0],
                    2 * NUMBER_OF_OFDM_CARRIERS,
                    2,
                    1);

#endif // DEBUG_PHY
#endif // USER_MODE

    } // antenna loop
#ifndef USER_MODE    
      time_out = openair_get_mbox();
#endif //USER_MODE      

/*       if ((mac_xface->frame % 100) == 0) */
/*        	      msg("[PHY][CHANNEL ESTIMATION][EMOS] Frame %d: phy_channel_estimation: time in %d, time out %d\n", */
/*        		  mac_xface->frame, */
/* 		  time_in, */
/* 		  time_out); */
	} 
}
