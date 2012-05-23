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
/*! \file log_if.h
* \brief log interface
* \author Navid Nikaein
* \date 2009
* \version 0.3
* \warning This component can be runned only in user-space
* @ingroup routing
*/
#ifndef __LOG_IF_H__
#    define __LOG_IF_H__


/*--- INCLUDES ---------------------------------------------------------------*/
#    include "log.h"
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#    ifdef COMPONENT_LOG
#        ifdef COMPONENT_LOG_IF
#            define private_log_if(x) x
#            define friend_log_if(x) x
#            define public_log_if(x) x
#        else
#            define private_log_if(x)
#            define friend_log_if(x) extern x
#            define public_log_if(x) extern x
#        endif
#    else
#        define private_log_if(x)
#        define friend_log_if(x)
#        define public_log_if(x) extern x
#    endif

/** @defgroup _log_if Interfaces of LOG
 * @{*/

//public_log_if( log_t *g_log;)

public_log_if( void logInit (void);)
public_log_if( void logRecord(const char *file, const char *func, int line,int comp, int level, char *format, ...);)
public_log_if( int set_comp_log(int component, int level, int verbosity, int interval);)
public_log_if( int  set_log(int component, int level, int interval);)
public_log_if( void set_glog(int level, int verbosity);)
public_log_if( void set_log_syslog(int enable);)
public_log_if( void set_log_onlinelog(int enable);)
public_log_if( void set_log_filelog(int enable);)
public_log_if( int  map_str_to_int(mapping *map, const char *str);)
public_log_if( char *map_int_to_str(mapping *map, int val);)
public_log_if( void logClean (void); )
public_log_if( int is_newline( char *str, int size);)

/* @}*/

#ifdef __cplusplus
}
#endif

#endif

