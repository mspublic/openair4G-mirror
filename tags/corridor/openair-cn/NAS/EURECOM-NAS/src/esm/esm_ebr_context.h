/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      esm_ebr_context.h

Version     0.1

Date        2013/05/28

Product     NAS stack

Subsystem   EPS Session Management

Author      Frederic Maurel

Description Defines functions used to handle EPS bearer contexts.

*****************************************************************************/
#ifndef __ESM_EBR_CONTEXT_H__
#define __ESM_EBR_CONTEXT_H__

#include "networkDef.h"

/****************************************************************************/
/*********************  G L O B A L    C O N S T A N T S  *******************/
/****************************************************************************/

/****************************************************************************/
/************************  G L O B A L    T Y P E S  ************************/
/****************************************************************************/

#ifdef NAS_UE
/* Traffic flow template operation */
typedef enum {
    ESM_EBR_CONTEXT_TFT_CREATE,
    ESM_EBR_CONTEXT_TFT_DELETE,
    ESM_EBR_CONTEXT_TFT_ADD_PACKET,
    ESM_EBR_CONTEXT_TFT_REPLACE_PACKET,
    ESM_EBR_CONTEXT_TFT_DELETE_PACKET,
} esm_ebr_context_tft_t;
#endif

/****************************************************************************/
/********************  G L O B A L    V A R I A B L E S  ********************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

#ifdef NAS_UE
int esm_ebr_context_create(int pid, int ebi, int is_default,
                           const network_qos_t *qos, const network_tft_t *tft);

int esm_ebr_context_release(int ebi, int *pid, int *bid);

int esm_ebr_context_get_pid(int ebi);

int esm_ebr_context_check_tft(int pid, int ebi, const network_tft_t *tft,
                              esm_ebr_context_tft_t operation);
#endif

#ifdef NAS_MME
int esm_ebr_context_create(emm_data_context_t *ctx, int pid, int ebi, int is_default,
                           const network_qos_t *qos, const network_tft_t *tft);

int esm_ebr_context_release(emm_data_context_t *ctx, int ebi, int *pid, int *bid);
#endif

#endif /* __ESM_EBR_CONTEXT_H__ */
