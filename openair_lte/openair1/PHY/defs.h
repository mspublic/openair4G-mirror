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
#include "impl_defs_lte.h"
#include "impl_defs_top.h"
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


#ifndef USER_MODE
#define debug_msg if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 10)) fifo_printf
#else
#define debug_msg msg
#endif

#endif






