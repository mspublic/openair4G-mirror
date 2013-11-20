/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		user_indication.c

Version		0.1

Date		2012/10/25

Product		NAS stack

Subsystem	Application Programming Interface

Author		Frederic Maurel

Description	Defines functions which allow the user application to register
		procedures to be executed upon receiving asynchronous notifi-
		cation.

*****************************************************************************/

#include "user_indication.h"
#include "nas_log.h"

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/* Notification handler */
static struct {
    unsigned char id;
    unsigned char mask;
    user_ind_callback_t callback[USER_IND_MAX];
} _user_ind_handler = {};

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:	 user_ind_register()                                       **
 **                                                                        **
 ** Description: Register the specified procedure to be executed upon      **
 **		 receiving the given asynchronous notification             **
 **                                                                        **
 ** Inputs:	 ind:		The notification type                      **
 **		 id:		The notification identifier                **
 **		 cb:		The procedure to register                  **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_user_ind_handler                          **
 **                                                                        **
 ***************************************************************************/
int user_ind_register(user_ind_t ind, unsigned char id, user_ind_callback_t cb)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    if (ind < USER_IND_MAX) {
	/* Register the notification callback */
	if (cb != NULL) {
	    _user_ind_handler.callback[ind] = cb;
	}
	else {
	    _user_ind_handler.id = id;
	    _user_ind_handler.mask |= (1 << ind);
	}
	rc = RETURNok;
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 user_ind_deregister()                                     **
 **                                                                        **
 ** Description: Prevent a registered procedure from being executed upon   **
 **		 receiving the given asynchronous notification             **
 **                                                                        **
 ** Inputs:	 ind:		The notification identifier                **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_user_ind_handler                          **
 **                                                                        **
 ***************************************************************************/
int user_ind_deregister(user_ind_t ind)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    if (ind < USER_IND_MAX) {
	_user_ind_handler.mask &= ~(1 << ind);
	rc = RETURNok;
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 user_ind_notify()                                         **
 **                                                                        **
 ** Description: Execute the procedure if registered for the received      **
 **		 notification identifier                                   **
 **                                                                        **
 ** Inputs:	 ind:		The notification identifier                **
 **		 data:		Generic pointer to the notification data   **
 **		 size:		The size parameter of the registered call- **
 **				back procedure to execute                  **
 **		 Others:	_user_ind_handler                          **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int user_ind_notify(user_ind_t ind, const void* data, size_t size)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    if (ind < USER_IND_MAX) {
	if (_user_ind_handler.mask & (1 << ind)) {
	    /* Execute the notification callback */
	    user_ind_callback_t notify = _user_ind_handler.callback[ind];
	    if (notify != NULL) {
		rc = (*notify)(_user_ind_handler.id, data, size);
	    }
	}
	else {
	    /* Silently discard not registered notification */
	    rc = RETURNok;
	}
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

