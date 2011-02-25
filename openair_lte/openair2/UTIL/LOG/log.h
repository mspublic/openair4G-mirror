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


#define MAX_LOG_ITEM 50
#define MAX_LOG_TOTAL 1000


#ifndef LOG_EMERG
#	define	LOG_EMERG	0	/* system is unusable */
#endif
#ifndef LOG_ALERT
#	define	LOG_ALERT	1	/* action must be taken immediately */
#endif
#ifndef LOG_CRIT
#	define	LOG_CRIT	2	/* critical conditions */
#endif
#ifndef LOG_ERR
#	define	LOG_ERR		3	/* error conditions */
#endif
#ifndef LOG_WARNING
#	define	LOG_WARNING	4	/* warning conditions */
#endif
#ifndef LOG_NOTICE
#	define	LOG_NOTICE	5	/* normal but significant condition */
#endif
#ifndef LOG_INFO
#	define	LOG_INFO	6	/* informational */
#endif
#ifndef LOG_DEBUG
#	define	LOG_DEBUG	7	/* debug-level messages */
#endif
#ifndef LOG_TRACE
#	define	LOG_TRACE	8	/* debug-level messages */
#endif

#define NUM_LOG_LEVEL  9

/*! \brief Macro used to call tr_log_full_ex with file, function and line information
 *
 */
#ifdef USER_MODE
#define logIt(component, level, format, args...) do {logRecord(__FILE__, __FUNCTION__, __LINE__, component,level, format, ##args);} while(0);
#else
#define logIt(component, level, format, args...) do {logRecord(NULL, __FUNCTION__, __LINE__, component,level, format, ##args);} while(0);
#endif
  typedef enum {MIN_LOG_COMPONENTS=0, LOG, MAC, MAX_LOG_COMPONENTS} comp_name_t;

// debugging macros
//#ifdef USER_MODE
#define LOG_G(c, x...) logIt(c, LOG_EMERG, x)
#define LOG_A(c, x...) logIt(c, LOG_ALERT, x)
#define LOG_C(c, x...)  logIt(c, LOG_CRIT,  x)
#define LOG_E(c, x...) logIt(c, LOG_ERR, x)
#define LOG_W(c, x...)  logIt(c, LOG_WARNING, x)
#define LOG_N(c, x...)logIt(c, LOG_NOTICE, x)
#define LOG_I(c, x...)  logIt(c, LOG_INFO, x)
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

/// Macro to log a message with severity DEBUG when entering a function
#define LOG_ENTER(c) do {LOG_T(c, "Entering\n");}while(0)
/// Macro to log a message with severity TRACE when exiting a function
#define LOG_EXIT(c) do {LOG_T(c,"Exiting\n"); return;}while(0)
/// Macro to log a function exit, including integer value, then to return a value to the calling function
#define LOG_RETURN(c,x) do {uint32_t __rv;__rv=(unsigned int)(x);LOG_T(c,"Returning %08x\n", __rv);return((typeof(x))__rv);}while(0)



/* .log_format = 0x13 uncolored standard messages
 * .log_format = 0x93 colored standard messages */

/// VT100 sequence for bold red foreground
#define LOG_RED "\033[1;31m"

/// VT100 sequence for green foreground
#define LOG_GREEN "\033[32m"

/// VT100 sequence for blue foreground
#define LOG_BLUE "\033[34m"

/// VT100 sequence for cyan foreground on black background
#define LOG_CYBL "\033[40;36m"

/// VT100 sequence for reset (black) foreground
#define LOG_RESET "\033[0m"

/// Optional start-format strings for highlighting
static char *log_level_highlight_start[] = {LOG_RED, LOG_RED, LOG_RED, LOG_RED, LOG_BLUE, "", "", "", LOG_GREEN};

/// Optional end-format strings for highlighting
static char *log_level_highlight_end[]   = {LOG_RESET, LOG_RESET, LOG_RESET, LOG_RESET, LOG_RESET, "", "", "", LOG_RESET};

/*** used to write lines (local/remote) in syslog.conf***/
#define LOG_LOCAL      0x01
#define LOG_REMOTE     0x02

#define FLAG_COLOR     0x001 //* defaults
#define FLAG_PID       0x002 //* defaults
#define FLAG_COMP      0x004
#define FLAG_THREAD    0x008 // all : 255/511
#define FLAG_LEVEL     0x010
#define FLAG_FUNCT     0x020
#define FLAG_FILE_LINE 0x040
#define FLAG_ONLINE    0X080 // online printing 
#define FLAG_LOG_TRACE 0x100 


#define LOG_DISABLE     0x00
#define LOG_MED         0x34
#define LOG_DEF         0x74
#define LOG_DEF_ONLINE  0xF4 // compatibility with OAI



#define OAI_OK 0                        ///< all ok
#define OAI_ERR 1                       ///< generic error
#define OAI_ERR_READ_ONLY 2             ///< tried to write to read-only item
#define OAI_ERR_NOTFOUND 3              ///< something wasn't found


  //#define msg printf


typedef struct  {
    const char *name;
    int level;
    int flag;
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
  int                     flag;
  int                     syslog;
  char*                   log_file_name;
} log_t;

/*--- INCLUDES ---------------------------------------------------------------*/
#    include "log_if.h"
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif


