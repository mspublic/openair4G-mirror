#include "UTIL/LOG/log_if.h"
#include "UTIL/OCG/OCG.h"
#include "UTIL/OPT/opt.h" // to test OPT
#include "UTIL/OMG/omg.h"
#include "SIMULATION/ETH_TRANSPORT/defs.h"

void oaisim_config(OAI_Emulation * emulation_scen, u16 * n_frames, char * g_log_level);

void olg_config(char * g_log_level);
void omg_config();
void ocg_config(OAI_Emulation * emulation_scen, u16 * n_frames);
void opt_config();
void otg_config();

void set_envi(OAI_Emulation * emulation_scen);
void set_topo(OAI_Emulation * emulation_scen, emu_info_t * emu_info);
void set_app(OAI_Emulation * emulation_scen);
void set_emu(OAI_Emulation * emulation_scen, u16 * n_frames);
