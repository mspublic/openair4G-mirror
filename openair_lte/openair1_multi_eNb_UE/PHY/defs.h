#ifndef __PHY_DEFS__H__
#define __PHY_DEFS__H__

#ifdef USER_MODE
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#define msg printf   
//use msg in the real-time thread context
#define msg_nrt printf   
//use msg_nrt in the non real-time context (for initialization, ...)
#ifdef EXPRESSMIMO_TARGET
#define malloc16(x) malloc(x)
#else //EXPRESSMIMO_TARGET
#define malloc16(x) memalign(16,x)
#endif //EXPRESSMIMO_TARGET
#define free16(y,x) free(y)
#define bigmalloc malloc
#define bigmalloc16 malloc16
#define openair_free(y,x) free((y))
#define PAGE_SIZE 4096

#define PAGE_MASK 0xfffff000
#define virt_to_phys(x) (x)

#define openair_sched_exit() exit(-1)

#define max(a,b)  ((a>b) ? (a) : (b))
#define min(a,b)  ((a<b) ? (a) : (b))

#else // USER_MODE
#include "ARCH/COMMON/defs.h"

#include <asm/io.h>
#include <asm/rtai.h>

#include <rtai.h>
#include <rtai_posix.h>
#include <rtai_fifos.h>
#include <rtai_math.h>

#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"

#define msg fifo_printf//rt_printk
#define msg_nrt printk

#ifdef BIGPHYSAREA

#define bigmalloc(x) (bigphys_malloc(x))
#define bigmalloc16(x) (bigphys_malloc(x))

#define malloc16(x) (bigphys_malloc(x))
#define free16(y,x) 

#define bigfree(y,x) 

#else // BIGPHYSAREA
 
#define bigmalloc(x) (dma_alloc_coherent(pdev[0],(x),&dummy_dma_ptr,0))
#define bigmalloc16(x) (dma_alloc_coherent(pdev[0],(x),&dummy_dma_ptr,0))
#define bigfree(y,x) (dma_free_coherent(pdev[0],(x),(void *)(y),dummy_dma_ptr))
#define malloc16(x) (kmalloc(x,GFP_KERNEL))
#define free16(y,x) (kfree(y))

#endif // BIGPHYSAREA


#ifdef CBMIMO1
#define openair_get_mbox() (*(unsigned int *)mbox)
#else //CBMIMO1
#define openair_get_mbox() (*(unsigned int *)PHY_vars->mbox>>1)
#endif //CBMIMO1

#endif // USERMODE

#define bzero(s,n) (memset((s),0,(n)))

#define cmax(a,b)  ((a>b) ? (a) : (b))
#define cmin(a,b)  ((a<b) ? (a) : (b))


#ifdef EXPRESSMIMO_TARGET
#define Zero_Buffer(x,y) Zero_Buffer_nommx(x,y)
#endif //EXPRESSMiMO_TARGET


#ifdef OPENAIR_LTE
#include "spec_defs_top.h"
#include "impl_defs_top.h"
#include "impl_defs_lte.h"
#else //OPENAIR_LTE
#include "spec_defs.h"
#include "impl_defs.h"
#endif //OPENAIR_LTE

#include "PHY/INIT/defs.h"
#include "PHY/CODING/defs.h"
#include "PHY/TOOLS/defs.h"
#include "PHY/MODULATION/defs.h"
#ifndef OPENAIR_LTE
#include "PHY/TRANSPORT/defs.h"
#include "PHY/ESTIMATION/defs.h"
#else //OPENAIR_LTE
#include "PHY/LTE_TRANSPORT/defs.h"
#include "PHY/LTE_ESTIMATION/defs.h"
#include "PHY/LTE_REFSIG/defs.h"
#endif //OPENAIR_LTE

/// Top-level PHY Data Structure for eNB 
typedef struct
{
  /// ACQ Mailbox for harware synch
  unsigned int *mbox;                
  LTE_DL_FRAME_PARMS  lte_frame_parms;
  PHY_MEASUREMENTS PHY_measurements; /// Measurement variables 
  LTE_eNB_COMMON   lte_eNB_common_vars;
  LTE_eNB_ULSCH    *lte_eNB_ulsch_vars[NUMBER_OF_UE_MAX];
  LTE_eNb_DLSCH_t  *dlsch_eNb[NUMBER_OF_UE_MAX][2];
  LTE_eNb_ULSCH_t  *ulsch_eNb[NUMBER_OF_UE_MAX];
  LTE_eNb_DLSCH_t  *dlsch_eNb_cntl;

  unsigned char    is_secondary_eNb; // primary by default
  unsigned char    is_init_sync;     /// Flag to tell if initial synchronization is performed. This affects how often the secondary eNb will listen to the PSS from the primary system.
  unsigned char    is_ready_to_communicate; /// Flag to tell if secondary eNb has channel estimates to create NULL-beams from.
  unsigned char    PeNb_id;          /// id of Primary eNb
  int              rx_offset;        /// Timing offset (used if is_secondary_eNb)

} PHY_VARS_eNB;

/// Top-level PHY Data Structure for UE 
typedef struct
{
  //PHY_MEASUREMENTS PHY_measurements; /// Measurement variables 
  LTE_DL_FRAME_PARMS  lte_frame_parms;
  LTE_UE_COMMON    lte_ue_common_vars;
  LTE_UE_DLSCH     *lte_ue_dlsch_vars[NUMBER_OF_eNB_MAX];
  LTE_UE_DLSCH     *lte_ue_dlsch_vars_cntl[NUMBER_OF_eNB_MAX];
  //  LTE_UE_DLSCH     *lte_ue_dlsch_vars_ra[NUMBER_OF_eNB_MAX];
  LTE_UE_PBCH      *lte_ue_pbch_vars[NUMBER_OF_eNB_MAX];
  LTE_UE_PDCCH     *lte_ue_pdcch_vars[NUMBER_OF_eNB_MAX];
  LTE_UE_DLSCH_t   *dlsch_ue[NUMBER_OF_eNB_MAX][2];
  LTE_UE_ULSCH_t   *ulsch_ue[NUMBER_OF_eNB_MAX];
  LTE_UE_DLSCH_t   *dlsch_ue_cntl;

} PHY_VARS_UE;

#endif






