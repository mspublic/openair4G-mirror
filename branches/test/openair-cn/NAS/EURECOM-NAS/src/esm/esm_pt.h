/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		esm_pt.h

Version		0.1

Date		2013/01/03

Product		NAS stack

Subsystem	EPS Session Management

Author		Frederic Maurel

Description	Defines functions used to handle ESM procedure transactions.

*****************************************************************************/
#ifndef __ESM_PT_H__
#define __ESM_PT_H__

#ifdef NAS_UE
#include "OctetString.h"
#include "nas_timer.h"
#endif

#include "ProcedureTransactionIdentity.h"

/****************************************************************************/
/*********************  G L O B A L    C O N S T A N T S  *******************/
/****************************************************************************/

/* Unassigned procedure transaction identity value */
#define ESM_PT_UNASSIGNED	(PROCEDURE_TRANSACTION_IDENTITY_UNASSIGNED)

/****************************************************************************/
/************************  G L O B A L    T Y P E S  ************************/
/****************************************************************************/

#ifdef NAS_UE
/* Procedure transaction states */
typedef enum {
    ESM_PT_INACTIVE,	/* No procedure transaction exists		*/
    ESM_PT_PENDING,	/* The UE has initiated a procedure transaction
			 * towards the network				*/
    ESM_PT_STATE_MAX
} esm_pt_state;

/* ESM message timer retransmission data */
typedef struct {
    unsigned char pti;		/* Procedure transaction identity	*/
    unsigned int count;		/* Retransmission counter		*/
    OctetString msg;		/* Encoded ESM message to re-transmit	*/
} esm_pt_timer_data_t;
#endif // NAS_UE

/****************************************************************************/
/********************  G L O B A L    V A R I A B L E S  ********************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

int esm_pt_is_reserved(int pti);

#ifdef NAS_UE
void esm_pt_initialize(void);

int esm_pt_assign(void);
int esm_pt_release(int pti);

int esm_pt_start_timer(int pti, const OctetString* msg, long sec, nas_timer_callback_t cb);
int esm_pt_stop_timer(int pti);

int esm_pt_set_status(int pti, esm_pt_state status);
esm_pt_state esm_pt_get_status(int pti);
int esm_pt_get_pending_pti(esm_pt_state status);

int esm_pt_is_not_in_use(int pti);
#endif // NAS_UE

#endif /* __ESM_PT_H__*/
