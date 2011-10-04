/*********************************************************************
                          rrc_ue_vars.h  -  description
                             -------------------
    copyright            : (C) 2005, 2008 by Eurecom
    email                : Michelle.Wetterwald@eurecom.fr
 *********************************************************************
  Define control block memory for RRC_UE
 ********************************************************************/
#ifndef __RRC_UE_VARS_H__
#define __RRC_UE_VARS_H__

#include "COMMON/mac_rrc_primitives.h"
#include "platform_types.h"
#include "mem_pool.h"

typedef mem_block_t mem_block;

#include "rrc_constant.h"
#include "rrc_ue_entity.h"
//-----------------------------------------------------------------------------
struct protocol_pool_ms {
  struct rrc_ms_entity rrc;
};

volatile struct protocol_pool_ms prot_pool_ms;
volatile struct protocol_pool_ms *protocol_ms;
RRC_XFACE *Rrc_xface;

int rrc_release_all_ressources;


#endif
