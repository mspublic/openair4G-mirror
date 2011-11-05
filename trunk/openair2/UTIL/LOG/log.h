/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file log.h
* \brief log file for openair
* \author Navid Nikaein
* \date 2009
* \version 0.3
* \warning This component can be runned only in user-space
* @ingroup routing

*/

/* for any problem of doxygen in this file, 
contact Lusheng Wang by lusheng.wang@eurecom.fr */

#ifndef __LOG_H__
#    define __LOG_H__

/*--- INCLUDES ---------------------------------------------------------------*/
#ifdef USER_MODE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#else
#include "rtai_fifos.h"
#endif

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup _LOG LOG Generator
 * @{*/ 
/* @}*/ 

/** @defgroup _macro Macro Definition
 *  @ingroup _LOG
 *  @brief these macros are used in the code of LOG
 * @{*/
/* @}*/ 

/** @defgroup _max_length Maximum Length of LOG
 *  @ingroup _macro
 *  @brief the macros that describe the maximum length of LOG
 * @{*/
#define MAX_LOG_ITEM 50 /*!< \brief the maximum length of a LOG item, what is LOG_ITEM ??? */
#define MAX_LOG_TOTAL 1000 /*!< \brief the maximum length of a log */
/* @}*/ 

/** @defgroup _log_level Message levels defined by LOG
 *  @ingroup _macro
 *  @brief LOG defines 9 levels of messages for users. Importance of these levels decrease gradually from 0 to 8
 * @{*/
#ifndef LOG_EMERG
#	define	LOG_EMERG	0	/*!< \brief system is unusable */
#endif
#ifndef LOG_ALERT
#	define	LOG_ALERT	1	/*!< \brief action must be taken immediately */
#endif
#ifndef LOG_CRIT
#	define	LOG_CRIT	2	/*!< \brief critical conditions */
#endif
#ifndef LOG_ERR
#	define	LOG_ERR		3	/*!< \brief error conditions */
#endif
#ifndef LOG_WARNING
#	define	LOG_WARNING	4	/*!< \brief warning conditions */
#endif
#ifndef LOG_NOTICE
#	define	LOG_NOTICE	5	/*!< \brief normal but significant condition */
#endif
#ifndef LOG_INFO
#	define	LOG_INFO	6	/*!< \brief informational */
#endif
#ifndef LOG_DEBUG
#	define	LOG_DEBUG	7	/*!< \brief debug-level messages */
#endif
#ifndef LOG_TRACE
#	define	LOG_TRACE	8	/*!< \brief debug-level messages */
#endif

#define NUM_LOG_LEVEL  9	/*!< \brief the number of message levels users have with LOG */
/* @}*/ 


/** @defgroup _logIt logIt function
 *  @ingroup _macro
 *  @brief Macro used to call tr_log_full_ex with file, function and line information
 * @{*/
#ifdef USER_MODE
#define logIt(component, level, format, args...) do {logRecord(__FILE__, __FUNCTION__, __LINE__, component, level, format, ##args);} while(0);
#else
#define logIt(component, level, format, args...) do {logRecord(NULL, __FUNCTION__, __LINE__, component, level, format, ##args);} while(0);
#endif
/* @}*/ 


/** @defgroup _debugging debugging macros
 *  @ingroup _macro
 *  @brief Macro used to call logIt function with different message levels
 * @{*/

// debugging macros
//#ifdef USER_MODE
#define LOG_G(c, x...) logIt(c, LOG_EMERG, x)
#define LOG_A(c, x...) logIt(c, LOG_ALERT, x)
#define LOG_C(c, x...) logIt(c, LOG_CRIT,  x)
#define LOG_E(c, x...) logIt(c, LOG_ERR, x)
#define LOG_W(c, x...) logIt(c, LOG_WARNING, x)
#define LOG_N(c, x...) logIt(c, LOG_NOTICE, x)
#define LOG_I(c, x...) logIt(c, LOG_INFO, x)
#define LOG_D(c, x...) logIt(c, LOG_DEBUG, x)
#define LOG_T(c, x...) logIt(c, LOG_TRACE, x)
/*
#else
#define LOG_G(c, x...) msg(x)
#define LOG_A(c, x...) msg(x)
#define LOG_C(c, x...) msg(x)
#define LOG_E(c, x...) msg(x)
#define LOG_W(c, x...) msg(x)
#define LOG_N(c, x...) msg(x)
#define LOG_I(c, x...) msg(x)
#define LOG_D(c, x...) msg(x)
#define LOG_T(c, x...) msg(x)
#endif
*/
/* @}*/ 


/** @defgroup _useful_functions useful functions in LOG
 *  @ingroup _macro
 *  @brief Macro of some useful functions defined by LOG
 * @{*/
#define LOG_ENTER(c) do {LOG_T(c, "Entering\n");}while(0)	/*!< \brief Macro to log a message with severity DEBUG when entering a function */
#define LOG_EXIT(c) do {LOG_T(c,"Exiting\n"); return;}while(0)	/*!< \brief Macro to log a message with severity TRACE when exiting a function */
#define LOG_RETURN(c,x) do {uint32_t __rv;__rv=(unsigned int)(x);LOG_T(c,"Returning %08x\n", __rv);return((typeof(x))__rv);}while(0)	/*!< \brief Macro to log a function exit, including integer value, then to return a value to the calling function */
/* @}*/


/** @defgroup _log_format Defined log format
 *  @ingroup _macro
 *  @brief Macro of log formats defined by LOG
 * @{*/

/* .log_format = 0x13 uncolored standard messages
 * .log_format = 0x93 colored standard messages */

#define LOG_RED "\033[1;31m"	/*!< \brief VT100 sequence for bold red foreground */
#define LOG_GREEN "\033[32m"	/*!< \brief VT100 sequence for green foreground */
#define LOG_BLUE "\033[34m"	/*!< \brief VT100 sequence for blue foreground */
#define LOG_CYBL "\033[40;36m"	/*!< \brief VT100 sequence for cyan foreground on black background */
#define LOG_RESET "\033[0m"	/*!< \brief VT100 sequence for reset (black) foreground */
/* @}*/


/** @defgroup _syslog_conf Macros for write in syslog.conf
 *  @ingroup _macro
 *  @brief Macros used to write lines (local/remote) in syslog.conf
 * @{*/
#define LOG_LOCAL      0x01
#define LOG_REMOTE     0x02

#define FLAG_COLOR     0x001	/*!< \brief defaults */
#define FLAG_PID       0x002	/*!< \brief defaults */
#define FLAG_COMP      0x004
#define FLAG_THREAD    0x008	/*!< \brief all : 255/511 */
#define FLAG_LEVEL     0x010
#define FLAG_FUNCT     0x020
#define FLAG_FILE_LINE 0x040
#define FLAG_LOG_TRACE 0x100 

#define LOG_NONE        0x00
#define LOG_LOW         0x04
#define LOG_MED         0x34
#define LOG_FULL        0x74

#define OAI_OK 0		/*!< \brief all ok */
#define OAI_ERR 1		/*!< \brief generic error */
#define OAI_ERR_READ_ONLY 2	/*!< \brief tried to write to read-only item */
#define OAI_ERR_NOTFOUND 3	/*!< \brief something wasn't found */
/* @}*/


static char *log_level_highlight_start[] = {LOG_RED, LOG_RED, LOG_RED, LOG_RED, LOG_BLUE, "", "", "", LOG_GREEN};	/*!< \brief Optional start-format strings for highlighting */

static char *log_level_highlight_end[]   = {LOG_RESET, LOG_RESET, LOG_RESET, LOG_RESET, LOG_RESET, "", "", "", LOG_RESET};	/*!< \brief Optional end-format strings for highlighting */

  typedef enum {MIN_LOG_COMPONENTS=0, LOG, PHY, MAC, EMU, OCG, OMG,OPT,OTG, RLC, PERF,RB, MAX_LOG_COMPONENTS} comp_name_t;

  //#define msg printf

typedef struct {
    char *name;	/*!< \brief string name of item */
    int value;	/*!< \brief integer value of mapping */
} mapping;


typedef struct  {
    const char *name;
    int level;
    int flag;
    int interval;
}log_component_t;

typedef struct  {
    unsigned int remote_ip;
    unsigned int audit_ip;
    int  remote_level;
    int  facility;
    int  audit_facility;
    int  format;
}log_config_t;


typedef struct {
  log_component_t         log_component[MAX_LOG_COMPONENTS];
  log_config_t            config;
  char*                   level2string[NUM_LOG_LEVEL];
  int                     level;
  int                     onlinelog;
  int                     flag;
  int                     syslog;
  int                     filelog;
  char*                   filelog_name;
} log_t;



/*--- INCLUDES ---------------------------------------------------------------*/
#    include "log_if.h"
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif


