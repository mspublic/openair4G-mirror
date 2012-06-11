/*******************************************************************************
 *
 * Eurecom OpenAirInterface 3
 * Copyright(c) 2012 Eurecom
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information
 * Openair Admin: openair_admin@eurecom.fr
 * Openair Tech : openair_tech@eurecom.fr
 * Forums       : http://forums.eurecom.fsr/openairinterface
 * Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France
 *
 *******************************************************************************/
/*! \file mRALlteInt_main.h
 * \brief This file defines the prototypes of the functions for starting the module mRALlteInt.
 * \author BRIZZOLA Davide, GAUTHIER Lionel, MAUREL Frederic, WETTERWALD Michelle
 * \date 2012
 * \version
 * \note
 * \bug
 * \warning
 */

#ifndef __MRALLTEINT_MAIN_H__
#    define __MRALLTEINT_MAIN_H__
//-----------------------------------------------------------------------------
#        ifdef MRALLTEINT_MAIN_C
#            define private_mRALlteInt_main(x)    x
#            define protected_mRALlteInt_main(x)  x
#            define public_mRALlteInt_main(x)     x
#        else
#            ifdef MRAL_MODULE
#                define private_mRALlteInt_main(x)
#                define protected_mRALlteInt_main(x)  extern x
#                define public_mRALlteInt_main(x)     extern x
#            else
#                define private_mRALlteInt_main(x)
#                define protected_mRALlteInt_main(x)
#                define public_mRALlteInt_main(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <libgen.h>
//-----------------------------------------------------------------------------
#include "mRALlteInt_mih_msg.h"
//-----------------------------------------------------------------------------

/*! \fn int mRALlteInt_main_usage(void)
 * \brief    Show usage of the LOG module.
 */
public_mRALlteInt_main( int mRALlteInt_main_usage(void);)

/*! \fn int mRALlteInt_main_init(unsigned int levelP)
 * \brief    Free all memory resources allocated and kept by this RLC AM instance.
 * \param[in]  levelP                    Level of log (ERROR, DEBUG).
 */
public_mRALlteInt_main( int mRALlteInt_main_init(unsigned int levelP);)

/*! \fn int mRALlteInt_main_record(unsigned int levelP, char * log_msgP, ...)
 * \brief    Set RLC AM protocol parameters.
 * \param[in]  levelP                     Level of log (ERROR, DEBUG).
 * \param[in]  log_msgP                   Message to log.
 */
public_mRALlteInt_main( int mRALlteInt_main_record(int level, const char * log_msg, ...);)

/*! \fn int mRALlteInt_main_exit(void)
 * \brief    Close, and clean properly the log module.
 */
public_mRALlteInt_main( int mRALlteInt_main_exit(void);)
#endif






