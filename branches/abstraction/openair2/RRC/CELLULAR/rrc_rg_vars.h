/*********************************************************************
                          rrc_rg_vars.h  -  description
                             -------------------
    copyright            : (C) 2005, 2008 by Eurecom
    email                : Michelle.Wetterwald@eurecom.fr
 *********************************************************************
  Define control block memory for RRC_UE
 ********************************************************************/
#ifndef __RRC_RG_VARS_H__
#define __RRC_RG_VARS_H__

#include "COMMON/mac_rrc_primitives.h"
#include "platform_types.h"
#include "mem_pool.h"

typedef mem_block_t mem_block;

#include "rrc_constant.h"
#include "rrc_rg_entity.h"
//-----------------------------------------------------------------------------
struct protocol_pool_bs {
  struct rrc_bs_entity rrc;
};

volatile struct protocol_pool_bs prot_pool_bs;
volatile struct protocol_pool_bs *protocol_bs;
RRC_XFACE *Rrc_xface;
int rrc_release_all_ressources;


#endif
