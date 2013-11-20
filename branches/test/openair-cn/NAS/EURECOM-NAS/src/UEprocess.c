/*****************************************************************************
 Eurecom OpenAirInterface 3
 Copyright(c) 2012 Eurecom

 Source		UEprocess.c

 Version		0.1

 Date		2012/02/27

 Product		NAS stack

 Subsystem	UE NAS main process

 Author		Frederic Maurel

 Description	Implements the Non-Access Stratum protocol for Evolved Packet
 system (EPS) running at the User Equipment side.

 *****************************************************************************/

#include "commonDef.h"
#include "nas_log.h"
#include "nas_timer.h"

#include "user_api.h"
#include "network_api.h"
#include "nas_user.h"
#include "nas_network.h"
#include "nas_parser.h"

#include <stdlib.h>	// exit
#include <poll.h>	// poll
#include <string.h>	// memset
#include <signal.h>	// sigaction
#include <pthread.h>

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

#define NAS_SLEEP_TIMEOUT 1000	/* 1 second */

static void* _nas_user_mngr(void*);
static void* _nas_network_mngr(void*);

static int _nas_set_signal_handler(int signal, void (handler)(int));
static void _nas_signal_handler(int signal);

static void _nas_clean(int usr_fd, int net_fd);

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/****************************************************************************/
int main(int argc, const char* argv[])
{
    /*
     * Get the command line options
     */
    if (nas_parser_get_options (argc, argv) != RETURNok)
    {
        nas_parser_print_usage (FIRMWARE_VERSION);
        exit (EXIT_FAILURE);
    }

    /*
     * Initialize logging trace utility
     */
    nas_log_init (nas_parser_get_trace_level ());

    const char* uhost = nas_parser_get_user_host ();
    const char* uport = nas_parser_get_user_port ();
    const char* devpath = nas_parser_get_device_path ();
    const char* devparams = nas_parser_get_device_params ();
    const char* nhost = nas_parser_get_network_host ();
    const char* nport = nas_parser_get_network_port ();

    LOG_TRACE (INFO, "UE-MAIN   - %s -ueid %d -uhost %s -uport %s -nhost %s -nport %s -dev %s -params %s -trace 0x%x",
               argv[0], nas_parser_get_ueid (), uhost, uport, nhost, nport, devpath, devparams,
               nas_parser_get_trace_level ());

    /*
     * Initialize the User interface
     */
    if (user_api_initialize (uhost, uport, devpath, devparams) != RETURNok)
    {
        LOG_TRACE (ERROR, "UE-MAIN   - user_api_initialize() failed");
        exit (EXIT_FAILURE);
    }
    int user_fd = user_api_get_fd ();

    /*
     * Initialize the Network interface
     */
    if (network_api_initialize (nhost, nport) != RETURNok)
    {
        LOG_TRACE (ERROR, "UE-MAIN   - network_api_initialize() failed");
        user_api_close (user_fd);
        exit (EXIT_FAILURE);
    }
    int network_fd = network_api_get_fd ();

    /*
     * Initialize the NAS contexts
     */
    nas_user_initialize (&user_api_emm_callback, &user_api_esm_callback, FIRMWARE_VERSION);
    nas_network_initialize ();

    /*
     * Initialize NAS timer handlers
     */
    nas_timer_init ();

    /*
     * Set up signal handlers
     */
    (void) _nas_set_signal_handler (SIGINT, _nas_signal_handler);
    (void) _nas_set_signal_handler (SIGTERM, _nas_signal_handler);

    pthread_attr_t attr;
    pthread_attr_init (&attr);
    pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

    /*
     * Start thread use to manage the user connection endpoint
     */
    pthread_t user_mngr;
    if (pthread_create (&user_mngr, &attr, _nas_user_mngr, &user_fd) != 0)
    {
        LOG_TRACE (ERROR, "UE-MAIN   - "
                   "Failed to create the user management thread");
        user_api_close (user_fd);
        network_api_close (network_fd);
        exit (EXIT_FAILURE);
    }

    /*
     * Start thread use to manage the network connection endpoint
     */
    pthread_t network_mngr;
    if (pthread_create (&network_mngr, &attr, _nas_network_mngr, &network_fd) != 0)
    {
        LOG_TRACE (ERROR, "UE-MAIN   - "
                   "Failed to create the network management thread");
        user_api_close (user_fd);
        network_api_close (network_fd);
        exit (EXIT_FAILURE);
    }
    pthread_attr_destroy (&attr);

    /*
     * Suspend execution of the main process until all connection
     * endpoints are still active
     */
    while ((user_fd != -1) && (network_fd != -1))
    {
        poll (NULL, 0, NAS_SLEEP_TIMEOUT);
        user_fd = user_api_get_fd ();
        network_fd = network_api_get_fd ();
    }

    /* Termination cleanup */
    _nas_clean (user_fd, network_fd);

    LOG_TRACE
    (WARNING, "UE-MAIN   - NAS main process exited");
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:	 _nas_user_mngr()                                          **
 **                                                                        **
 ** Description: Manages the connection endpoint use to communicate with   **
 **		 the user application layer                                **
 **                                                                        **
 ** Inputs:	 fd:		The descriptor of the user connection end- **
 **				point                                      **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 Return:	None                                       **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static void* _nas_user_mngr(void* args)
{
    LOG_FUNC_IN;

    int ret_code;
    int exit_loop = FALSE;
    int nb_command;
    int bytes;

    int *fd = (int*) args;

    LOG_TRACE (INFO, "UE-MAIN   - User connection manager started (%d)", *fd);

    /* User receiving loop */
    while (!exit_loop)
    {
        /* Read the user data message */
        bytes = user_api_read_data (*fd);
        if (bytes == RETURNerror)
        {
            /* Failed to read data from the user application layer;
             * exit from the receiving loop */
            LOG_TRACE (ERROR, "UE-MAIN   - "
                       "Failed to read data from the user application layer");
            break;
        }

        if (bytes == 0)
        {
            /* A signal was caught before any data were available */
            continue;
        }

        /* Decode the user data message */
        nb_command = user_api_decode_data (bytes);
        for (int i = 0; i < nb_command; i++)
        {
            /* Get the user data to be processed */
            const void* data = user_api_get_data (i);
            if (data == NULL)
            {
                /* Failed to get user data at the given index;
                 * go ahead and process the next user data */
                LOG_TRACE (ERROR, "UE-MAIN   - "
                           "Failed to get user data at index %d",
                           i);
                continue;
            }

            /* Process the user data message */
            ret_code = nas_user_process_data (data);
            if (ret_code != RETURNok)
            {
                /* The user data message has not been successfully
                 * processed; cause code will be encoded and sent back
                 * to the user */
                LOG_TRACE
                (WARNING, "UE-MAIN   - "
                 "The user procedure call failed");
            }

            /* Encode the user data message */
            bytes = user_api_encode_data (nas_user_get_data (), i == nb_command - 1);
            if (bytes == RETURNerror)
            {
                /* Failed to encode the user data message;
                 * go ahead and process the next user data */
                continue;
            }

            /* Send the data message to the user */
            bytes = user_api_send_data (*fd, bytes);
            if (bytes == RETURNerror)
            {
                /* Failed to send data to the user application layer;
                 * exit from the receiving loop */
                LOG_TRACE (ERROR, "UE-MAIN   - "
                           "Failed to send data to the user application layer");
                exit_loop = TRUE;
                break;
            }
        }
    }

    /* Close the connection to the user application layer */
    user_api_close (*fd);
    LOG_TRACE (WARNING, "UE-MAIN   - "
               "The user connection endpoint manager exited");

    LOG_FUNC_RETURN(NULL);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _nas_network_mngr()                                       **
 **                                                                        **
 ** Description: Manages the connection endpoint use to communicate with   **
 **		 the network sublayer                                      **
 **                                                                        **
 ** Inputs:	 fd:		The descriptor of the network connection   **
 **				endpoint                                   **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 Return:	None                                       **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static void* _nas_network_mngr(void* args)
{
    LOG_FUNC_IN;

    int ret_code;
    int network_message_id;
    int bytes;

    int *fd = (int*) args;

    LOG_TRACE (INFO, "UE-MAIN   - Network connection manager started (%d)", *fd);

    /* Network receiving loop */
    while (TRUE)
    {
        /* Read the network data message */
        bytes = network_api_read_data (*fd);
        if (bytes == RETURNerror)
        {
            /* Failed to read data from the network sublayer;
             * exit from the receiving loop */
            LOG_TRACE (ERROR, "UE-MAIN   - "
                       "Failed to read data from the network sublayer");
            break;
        }

        if (bytes == 0)
        {
            /* A signal was caught before any data were available */
            continue;
        }

        /* Decode the network data message */
        network_message_id = network_api_decode_data (bytes);
        if (network_message_id == RETURNerror)
        {
            /* Failed to decode data read from the network sublayer */
            continue;
        }

        /* Process the network data message */
        ret_code = nas_network_process_data (network_message_id, network_api_get_data ());
        if (ret_code != RETURNok)
        {
            /* The network data message has not been successfully
             * processed */
            LOG_TRACE
            (WARNING, "UE-MAIN   - "
             "The network procedure call 0x%x failed",
             network_message_id);
        }
    }

    /* Close the connection to the network sublayer */
    network_api_close (*fd);
    LOG_TRACE (WARNING, "UE-MAIN   - "
               "The network connection endpoint manager exited");

    LOG_FUNC_RETURN(NULL);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _nas_set_signal_handler()                                 **
 **                                                                        **
 ** Description: Set up a signal handler                                   **
 **                                                                        **
 ** Inputs:	 signal:	Signal number                              **
 ** 	 	 handler:	Signal handler                             **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 Return:	RETURNerror, RETURNok                      **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _nas_set_signal_handler(int signal, void (handler)(int))
{
    LOG_FUNC_IN;

    struct sigaction act;

    /* Initialize signal set */
    (void) memset (&act, 0, sizeof(act));
    (void) sigfillset (&act.sa_mask);
    (void) sigdelset (&act.sa_mask, SIGHUP);
    (void) sigdelset (&act.sa_mask, SIGINT);
    (void) sigdelset (&act.sa_mask, SIGTERM);
    (void) sigdelset (&act.sa_mask, SIGILL);
    (void) sigdelset (&act.sa_mask, SIGTRAP);
    (void) sigdelset (&act.sa_mask, SIGIOT);
#ifndef LINUX
    (void) sigdelset (&act.sa_mask, SIGEMT);
#endif
    (void) sigdelset (&act.sa_mask, SIGFPE);
    (void) sigdelset (&act.sa_mask, SIGBUS);
    (void) sigdelset (&act.sa_mask, SIGSEGV);
    (void) sigdelset (&act.sa_mask, SIGSYS);

    /* Initialize signal handler */
    act.sa_handler = handler;
    if (sigaction (signal, &act, 0) < 0)
    {
        return RETURNerror;
    }

    LOG_TRACE (INFO, "UE-MAIN   - Handler successfully set for signal %d", signal);

    LOG_FUNC_RETURN(RETURNok);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _nas_signal_handler()                                     **
 **                                                                        **
 ** Description: Signal handler                                            **
 **                                                                        **
 ** Inputs:	 signal:	Signal number                              **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 Return:	None                                       **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static void _nas_signal_handler(int signal)
{
    LOG_FUNC_IN;

    LOG_TRACE (WARNING, "UE-MAIN   - Signal %d received", signal);
    _nas_clean (user_api_get_fd (), network_api_get_fd ());
    exit (EXIT_SUCCESS);

    LOG_FUNC_OUT
    ;
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _nas_clean()                                              **
 **                                                                        **
 ** Description: Performs termination cleanup                              **
 **                                                                        **
 ** Inputs:	 usr_fd:	User's connection file descriptor          **
 **		 net_fd:	Network's connection file descriptor       **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 Return:	None                                       **
 ** 	 	 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static void _nas_clean(int usr_fd, int net_fd)
{
    LOG_FUNC_IN;

    LOG_TRACE (INFO, "UE-MAIN   - Perform EMM and ESM cleanup");
    nas_network_cleanup ();

    LOG_TRACE (INFO, "UE-MAIN   - "
               "Closing user connection %d and network connection %d",
               usr_fd, net_fd);
    user_api_close (usr_fd);
    network_api_close (net_fd);

    LOG_FUNC_OUT
    ;
}

