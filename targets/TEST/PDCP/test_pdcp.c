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

// Declare PDCP entities
pdcp_t pdcp_array[2];

int main(int argc, char **argv) {
  char c;
  char * g_log_level="trace"; // by default global log level is set to trace

  mac_xface = malloc(sizeof(MAC_xface));

  return 0;
}

