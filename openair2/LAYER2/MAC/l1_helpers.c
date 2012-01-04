#include "defs.h"
#include "extern.h"
#include "UTIL/LOG/log.h"

s8 get_Po_NOMINAL_PUSCH(u8 Mod_id) {
  RACH_ConfigCommon_t *rach_ConfigCommon = NULL;

  if (UE_mac_inst[Mod_id].radioResourceConfigCommon)
    rach_ConfigCommon = &UE_mac_inst[Mod_id].radioResourceConfigCommon->rach_ConfigCommon;
  else {
    LOG_D(MAC,"[UE %d] FATAL Frame %d: radioResourceConfigCommon is NULL !!!\n",Mod_id,mac_xface->frame);
    mac_xface->macphy_exit("");
  }

  return(-120 + (rach_ConfigCommon->powerRampingParameters.preambleInitialReceivedTargetPower<<1) + 
	 get_DELTA_PREAMBLE(Mod_id));
}

s8 get_deltaP_rampup(u8 Mod_id) {

  LOG_D(MAC,"[PUSCH]%d dB\n",UE_mac_inst[Mod_id].RA_PREAMBLE_TRANSMISSION_COUNTER<<1);
  return((s8)(UE_mac_inst[Mod_id].RA_PREAMBLE_TRANSMISSION_COUNTER<<1));
 
}
