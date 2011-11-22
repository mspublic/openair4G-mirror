#include <string.h>
#include <math.h>
#include <unistd.h>
#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"

#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/vars.h"
#include "UTIL/LOG/log_if.h"
#include "UTIL/LOG/log_extern.h"
#include "RRC/LITE/vars.h"
#include "PHY_INTERFACE/vars.h"
#include "UTIL/OCG/OCG.h"
#include "LAYER2/PDCP_v10.1.0/pdcp.h"
#include "OCG_vars.h"

int pdcp_fifo_flush_sdus () {}
int pdcp_fifo_read_input_sdus_remaining_bytes () {}
int pdcp_fifo_read_input_sdus () {}

BOOL init_pdcp_entity(pdcp_t *pdcp_entity);

// Declare PDCP entities
pdcp_t pdcp_array[2];

int main(int argc, char **argv) {
  unsigned long index = 0;
  char * g_log_level = "trace"; // by default global log level is set to trace

  mac_xface = malloc(sizeof(MAC_xface));

  if (init_pdcp_entity(&pdcp_array[0]) == TRUE && init_pdcp_entity(&pdcp_array[1]) == TRUE)
    msg("PDCP entity initialization OK\n");
  else {
    msg("Cannot initialize PDCP entities!\n");
    return 1;
  }

  /* Initialize PDCP state variables */
  for (index = 0; index < 2; ++index) {
    if (pdcp_init_seq_numbers(&pdcp_array[index]) == FALSE) {
      msg("Cannot initialize %s PDCP entity!\n", ((index == 0) ? "first" : "second"));
      exit(1);
    }
  }

  /* Test TX window */
  for (index = 0; index < 10000; ++index) {
    u16 pseudo_tx_sn = pdcp_get_next_tx_seq_number(&pdcp_array[0]);
    if (pseudo_tx_sn == index % 4096)
      msg("TX packet # %07lu seq # %04d hfn # %04d\n", index, pseudo_tx_sn, pdcp_array[0].tx_hfn);
    else {
      msg("TX packet is out-of-window!\n");
      exit(1);
    }
  }

  /* Test RX window */
  for (index = 0; index < 10000; ++index) {
    u16 pseudo_rx_sn = (index == 0) ? 0 : index % 4096;
    if (pdcp_is_rx_seq_number_valid(pseudo_rx_sn, &pdcp_array[1]) == TRUE) {
      msg("RX packet # %07lu seq # %04d last-submitted # %04d hfn # %04d\n", \
          index, pdcp_array[1].next_pdcp_rx_sn, pdcp_array[1].last_submitted_pdcp_rx_sn, pdcp_array[1].rx_hfn);
    } else {
      msg("RX packet seq # %04lu is not valid!\n", index);
      exit(1);
    }
  }

  return 0;
}

BOOL init_pdcp_entity(pdcp_t *pdcp_entity)
{
  if (pdcp_entity == NULL)
    return FALSE;

  /*
   * Initialize sequence number state variables of relevant PDCP entity
   */
  pdcp_entity->next_pdcp_tx_sn = 0;
  pdcp_entity->next_pdcp_rx_sn = 0;
  pdcp_entity->tx_hfn = 0;
  pdcp_entity->rx_hfn = 0;
  /* SN of the last PDCP SDU delivered to upper layers */
  pdcp_entity->last_submitted_pdcp_rx_sn = 4095;
  pdcp_entity->seq_num_size = 12;

  return TRUE;
}

