/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2010 Eurecom

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
/*! \file log.c
* \brief log implementaion
* \author Navid Nikaein
* \date 2009
* \version 0.3
* \warning This component can be runned only in user-space
* @ingroup routing

*/

//#define LOG_TEST 1

#define COMPONENT_LOG
#define COMPONENT_LOG_IF



#include "../include/log.h"

// made static and not local to logRecord() for performance reasons
static char g_buff_info [MAX_LOG_TOTAL];
static char g_buff_infos[MAX_LOG_TOTAL];
static char g_buff_tmp  [MAX_LOG_ITEM];
static char g_buff_debug[MAX_LOG_ITEM];

static int thread_safe_debug_count = 0;

static char copyright_string[] __attribute__ ((unused)) =
  "Copyright (c) The www.openairinterface.org  2009, navid nikaein (navid.nikaein@eurecom.fr) All rights reserved.";

static const char BUILD_VERSION[] = "v0.1";
static const char BUILD_DATE[] = "2011-01-15 16:42:14";
static const char BUILD_HOST[] = "LINUX";
static const char BUILD_TARGET[] = "OAI";

    log_t *g_log;

void logInit ()
{
    //printf("[LOG] logInit(%p)\n", this);
    g_log = calloc(1, sizeof(log_t));
    memset(g_log, 0, sizeof(log_t));

/*
    g_log->log_component[LOG].name = "LOG";
    g_log->log_component[LOG].level = LOG_TRACE;
    g_log->log_component[LOG].flag = LOG_DEF;

    g_log->log_component[MAC].name = "MAC";
    g_log->log_component[MAC].level = LOG_INFO;
    g_log->log_component[MAC].flag = LOG_DEF_ONLINE;
*/
    g_log->log_component[OCG].name = "OCG";
    g_log->log_component[OCG].level = LOG_INFO;
    g_log->log_component[OCG].flag = LOG_DEF_ONLINE;


    g_log->level2string[LOG_EMERG]         = "G"; //EMERG
    g_log->level2string[LOG_ALERT]         = "A"; // ALERT
    g_log->level2string[LOG_CRIT]          = "C"; // CRITIC
    g_log->level2string[LOG_ERR]           = "E"; // ERROR
    g_log->level2string[LOG_WARNING]       = "W"; // WARNING
    g_log->level2string[LOG_NOTICE]        = "N"; // NOTICE
    g_log->level2string[LOG_INFO]          = "I"; //INFO
    g_log->level2string[LOG_DEBUG]         = "D"; // DEBUG
    g_log->level2string[LOG_TRACE]         = "T"; // TRACE

    g_log->syslog = 0;
    g_log->level  = LOG_TRACE;
    g_log->flag    = LOG_DEF;
 
    g_log->config.remote_ip      = 0;
    g_log->config.remote_level   = LOG_EMERG;
    g_log->config.facility       = LOG_LOCAL7;
    g_log->config.audit_ip       = 0;
    g_log->config.audit_facility = LOG_LOCAL6;
    g_log->config.format         = 0x00; // online debug inactive

    g_log->log_file_name = "openair.log";
}

void logRecord( const char *file, const char *func,
		int line,  int comp, int level, 
		char *format, ...) {
   
  va_list args;
  log_component_t *c;
  
  thread_safe_debug_count++;
  g_buff_infos[0] = '\0';
  c = &g_log->log_component[comp];

  
  // only log messages which are enabled and are below the global log level and component's level threshold
  if (c->level > g_log->level || level > c->level ) {
    thread_safe_debug_count--;
      return;
    }
  
  // adjust syslog level for TRACE messages
  if (g_log->syslog) {
    if (g_log->level > LOG_DEBUG) {
      g_log->level = LOG_DEBUG;
    }  
  }
 
    va_start(args, format);
  vsnprintf(g_buff_info, MAX_LOG_TOTAL, format, args);
  va_end(args);


  if ( g_log->flag & FLAG_COLOR )  {
    snprintf(g_buff_tmp, MAX_LOG_ITEM, "%s",
	     log_level_highlight_start[g_log->level]);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
  
  if ( g_log->flag & FLAG_COMP ){
    snprintf(g_buff_tmp, MAX_LOG_ITEM, "[%s]",
	     g_log->log_component[comp].name);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
  
  if ( g_log->flag & FLAG_LEVEL ){
    snprintf(g_buff_tmp, MAX_LOG_ITEM, "[%s]",
	     g_log->level2string[level]);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
    
  if (  g_log->flag & FLAG_FUNCT )  {
    snprintf(g_buff_tmp, MAX_LOG_ITEM, "[%s ",
	     func);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
  
  if (  g_log->flag & FLAG_FILE_LINE )  {
    snprintf(g_buff_tmp, MAX_LOG_ITEM, "%s:%d]",
	     file,line);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
  
  if (  g_log->flag & FLAG_COLOR )  {
    snprintf(g_buff_tmp, MAX_LOG_ITEM, "%s", 
	     log_level_highlight_end[g_log->level]);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
 
  strncat(g_buff_infos, g_buff_info, MAX_LOG_TOTAL);
  //strncat(g_buff_infos, "\n", MAX_LOG_TOTAL);

  // OAI printf compatibility 
  if (g_log->flag & FLAG_ONLINE || c->flag & FLAG_ONLINE) 
    msg("%s",g_buff_infos);

  if (g_log->syslog) {
    openlog(c->name, LOG_PID, g_log->config.facility);
    syslog(g_log->level, g_buff_infos);
    closelog();
  } else {
    int fd;
    fd = open(g_log->log_file_name, O_WRONLY | O_CREAT | O_APPEND, 0666);
    write(fd, g_buff_infos, strlen(g_buff_infos));
    close(fd);
  }
  thread_safe_debug_count--;
}

int  set_comp_log(int component, int level, int flag) {
  if (component >=MIN_LOG_COMPONENTS && component < MAX_LOG_COMPONENTS){
    g_log->log_component[component].flag = flag;
    g_log->log_component[component].level = level;
    return 0;
  }
  else
    return 1;
}

void set_glog(int level, int flag) {
  g_log->level = level;
  g_log->flag = flag;
}
void set_log_syslog(int enable) {
  g_log->syslog = enable;
}

