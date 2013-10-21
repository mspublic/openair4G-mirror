/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		emm_reg.c

Version		0.1

Date		2012/10/16

Product		NAS stack

Subsystem	EPS Mobility Management

Author		Frederic Maurel

Description	Defines the EMMREG Service Access Point that provides
		registration services for location updating and attach/detach
		procedures.

*****************************************************************************/

#include "emm_reg.h"
#include "commonDef.h"
#include "nas_log.h"

#include "emm_fsm.h"

#include <assert.h>

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
 ** Name:	 emm_reg_initialize()                                      **
 **                                                                        **
 ** Description: Initializes the EMMREG Service Access Point               **
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	NONE                                       **
 **                                                                        **
 ***************************************************************************/
void emm_reg_initialize(void)
{
    LOG_FUNC_IN;

    /* Initialize the EMM state machine */
    emm_fsm_initialize();

    LOG_FUNC_OUT;
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_reg_send()                                            **
 **                                                                        **
 ** Description: Processes the EMMREG Service Access Point primitive       **
 **                                                                        **
 ** Inputs:	 msg:		The EMMREG-SAP primitive to process        **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_reg_send(const emm_reg_t* msg)
{
    LOG_FUNC_IN;

    int rc;

    /* Check the EMM-SAP primitive */
    emm_reg_primitive_t primitive = msg->primitive;
    assert( (primitive > _EMMREG_START) && (primitive < _EMMREG_END));

    /* Execute the EMM procedure */
    rc = emm_fsm_process(msg);

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

