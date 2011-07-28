#include "UTIL/LOG/log_if.h"
#include "UTIL/OCG/OCG.h"
#include "UTIL/OPT/opt.h" // to test OPT
#include "UTIL/OMG/omg.h"
#include "SIMULATION/ETH_TRANSPORT/defs.h"

void oaisim_config(u16 * n_frames, char * g_log_level);

void olg_config(char * g_log_level);
void ocg_config_omg(OAI_Emulation * emulation_scen);
void ocg_config(OAI_Emulation * emulation_scen, u16 * n_frames);
void ocg_config_opt(OAI_Emulation * emulation_scen);
void ocg_config_otg(OAI_Emulation * emulation_scen);

void config_omg();

void set_envi(OAI_Emulation * emulation_scen);
void set_topo(OAI_Emulation * emulation_scen);
void set_app(OAI_Emulation * emulation_scen);
void set_emu(OAI_Emulation * emulation_scen, u16 * n_frames);
