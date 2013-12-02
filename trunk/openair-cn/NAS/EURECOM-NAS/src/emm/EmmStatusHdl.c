/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      Emmstatus.c

Version     0.1

Date        2013/06/26

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Defines the EMM status procedure executed by the Non-Access
        Stratum.

        The purpose of the sending of the EMM STATUS message is to
        report at any time certain error conditions detected upon
        receipt of EMM protocol data. The EMM STATUS message can be
        sent by both the MME and the UE.

*****************************************************************************/

#include "emm_proc.h"
#include "commonDef.h"
#include "nas_log.h"

#include "emm_cause.h"
#include "emmData.h"

#include "emm_sap.h"

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:    emm_proc_status_ind()                                     **
 **                                                                        **
 ** Description: Processes received EMM status message.                    **
 **                                                                        **
 **      3GPP TS 24.301, section 5.7                               **
 **      On receipt of an EMM STATUS message no state transition   **
 **      and no specific action shall be taken. Local actions are  **
 **      possible and are implementation dependent.                **
 **                                                                        **
 ** Inputs:  ueid:      UE lower layer identifier                  **
 **          emm_cause: Received EMM cause code                    **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_proc_status_ind(unsigned int ueid, int emm_cause)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(INFO,"EMM-PROC  - EMM status procedure requested (cause=%d)",
              emm_cause);

    LOG_TRACE(DEBUG, "EMM-PROC  - To be implemented");

    /* TODO */
    rc = RETURNok;

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_proc_status()                                         **
 **                                                                        **
 ** Description: Initiates EMM status procedure.                           **
 **                                                                        **
 ** Inputs:  ueid:      UE lower layer identifier                  **
 **      emm_cause: EMM cause code to be reported              **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_proc_status(unsigned int ueid, int emm_cause)
{
    LOG_FUNC_IN;

    int rc;
    emm_sap_t emm_sap;

    emm_security_context_t    *sctx = NULL;
    struct emm_data_context_s *ctx  = NULL;

    LOG_TRACE(INFO,"EMM-PROC  - EMM status procedure requested");

    /*
     * Notity EMM that EMM status indication has to be sent to lower layers
     */
    emm_sap.primitive = EMMAS_STATUS_IND;
    emm_sap.u.emm_as.u.status.emm_cause = emm_cause;
    emm_sap.u.emm_as.u.status.ueid = ueid;

#ifdef NAS_UE
    emm_sap.u.emm_as.u.status.guti = _emm_data.guti;
    sctx = _emm_data.security;
#endif
#ifdef NAS_MME
    emm_sap.u.emm_as.u.status.guti = NULL;
# if defined(EPC_BUILD)
    ctx = emm_data_context_get(&_emm_data, ueid);
# else
    ctx = _emm_data.ctx[ueid];
# endif
    if (ctx) {
        sctx = ctx->security;
    }
#endif
    /* Setup EPS NAS security data */
    emm_as_set_security_data(&emm_sap.u.emm_as.u.status.sctx, sctx,
                             FALSE, TRUE);

    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/
