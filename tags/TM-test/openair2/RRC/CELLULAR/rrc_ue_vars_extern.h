/*********************************************************************
                          rrc_ue_vars_extern.h  -  description
                             -------------------
    copyright            : (C) 2005, 2008 by Eurecom
    email                : Michelle.Wetterwald@eurecom.fr
 *********************************************************************
  External definitions for control block memory for RRC_UE
 ********************************************************************/
#ifndef __RRC_UE_VARS_EXTERN_H__
#define __RRC_UE_VARS_EXTERN_H__

#include "COMMON/mac_rrc_primitives.h"
#include "rrc_constant.h"
#include "rrc_ue_entity.h"

#include "mem_pool.h"
//-----------------------------------------------------------------------------

extern volatile struct protocol_pool_ms *protocol_ms;
extern int      rrc_release_all_ressources;
extern RRC_XFACE *Rrc_xface;
#endif
