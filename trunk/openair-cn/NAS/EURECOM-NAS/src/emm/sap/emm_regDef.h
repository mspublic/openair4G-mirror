/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		emm_regDef.h

Version		0.1

Date		2012/10/16

Product		NAS stack

Subsystem	EPS Mobility Management

Author		Frederic Maurel

Description	Defines the EMMREG Service Access Point that provides
		registration services for location updating and attach/detach
		procedures.

*****************************************************************************/
#ifndef __EMM_REGDEF_H__
#define __EMM_REGDEF_H__

#include "commonDef.h"

/****************************************************************************/
/*********************  G L O B A L    C O N S T A N T S  *******************/
/****************************************************************************/

/*
 * EMMREG-SAP primitives
 */
typedef enum {
    _EMMREG_START = 0,
#ifdef NAS_UE
    _EMMREG_S1_ENABLED,	  /* S1 mode is enabled				*/
    _EMMREG_S1_DISABLED,  /* S1 mode is disabled			*/
    _EMMREG_NO_IMSI,	  /* The UE is powered on without a valid USIM	*/
    _EMMREG_NO_CELL,	  /* No available cell has been found		*/
    _EMMREG_REGISTER_REQ, /* Network registration procedure requested	*/
    _EMMREG_REGISTER_CNF, /* Network registration procedure succeed	*/
    _EMMREG_REGISTER_REJ, /* Network registration procedure rejected	*/
    _EMMREG_ATTACH_INIT,	/* EPS network attach initiated		*/
    _EMMREG_ATTACH_REQ,		/* EPS network attach requested		*/
    _EMMREG_ATTACH_FAILED,	/* Attach attempt failed		*/
    _EMMREG_ATTACH_EXCEEDED,	/* Attach attempt exceeded		*/
    _EMMREG_AUTH_REJ,		/* Authentication rejected		*/
#endif
#ifdef NAS_MME
    _EMMREG_COMMON_PROC_REQ,	/* EMM common procedure requested	*/
    _EMMREG_COMMON_PROC_CNF,	/* EMM common procedure successful	*/
    _EMMREG_COMMON_PROC_REJ,	/* EMM common procedure failed		*/
    _EMMREG_PROC_ABORT,		/* EMM procedure aborted		*/
#endif
    _EMMREG_ATTACH_CNF,		/* EPS network attach accepted		*/
    _EMMREG_ATTACH_REJ,		/* EPS network attach rejected		*/
    _EMMREG_DETACH_INIT,	/* Network detach initiated		*/
    _EMMREG_DETACH_REQ,		/* Network detach requested		*/
    _EMMREG_DETACH_FAILED,	/* Network detach attempt failed	*/
    _EMMREG_DETACH_CNF,		/* Network detach accepted		*/
    _EMMREG_TAU_REQ,
    _EMMREG_TAU_CNF,
    _EMMREG_TAU_REJ,
    _EMMREG_SERVICE_REQ,
    _EMMREG_SERVICE_CNF,
    _EMMREG_SERVICE_REJ,
    _EMMREG_LOWERLAYER_SUCCESS,	/* Data successfully delivered		*/
    _EMMREG_LOWERLAYER_FAILURE,	/* Lower layer failure indication	*/
    _EMMREG_LOWERLAYER_RELEASE,	/* NAS signalling connection released	*/
    _EMMREG_END
} emm_reg_primitive_t;

/****************************************************************************/
/************************  G L O B A L    T Y P E S  ************************/
/****************************************************************************/

#ifdef NAS_UE
/*
 * EMMREG primitive for network registration procedure
 * ----------------------------------------------------
 */
typedef struct {
    int index;		/* Index of the currently selected PLMN in the ordered
			 * list of available PLMNs */
} emm_reg_register_t;
#endif

/*
 * EMMREG primitive for attach procedure
 * -------------------------------------
 */
typedef struct {
    int is_emergency;	/* TRUE if the UE was attempting to register to
			 * the network for emergency services only	*/
} emm_reg_attach_t;

/*
 * EMMREG primitive for detach procedure
 * -------------------------------------
 */
typedef struct {
    int switch_off;	/* TRUE if the UE is switched off	 	*/
    int type;		/* Network detach type				*/
} emm_reg_detach_t;

#ifdef NAS_MME
/*
 * EMMREG primitive for EMM common procedures
 * ------------------------------------------
 */
typedef struct {
    int is_attached;	/* UE context attach indicator			*/
} emm_reg_common_t;
#endif

/*
 * Structure of EMMREG-SAP primitive
 */
typedef struct {
    emm_reg_primitive_t primitive;
    unsigned int        ueid;
    void               *ctx;

    union {
	emm_reg_attach_t	attach;
	emm_reg_detach_t	detach;
#ifdef NAS_UE
	emm_reg_register_t	regist;
#endif
#ifdef NAS_MME
	emm_reg_common_t	common;
#endif
    } u;
} emm_reg_t;

/****************************************************************************/
/********************  G L O B A L    V A R I A B L E S  ********************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

#endif /* __EMM_REGDEF_H__*/
