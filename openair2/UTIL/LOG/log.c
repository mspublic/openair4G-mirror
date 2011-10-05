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
* \date 2011
* \version 0.5
* \warning This component can be run only in user-space
* @ingroup routing

*/

//#define LOG_TEST 1

#define COMPONENT_LOG
#define COMPONENT_LOG_IF



//static unsigned char       fifo_print_buffer[FIFO_PRINTF_MAX_STRING_SIZE];

#include "log.h"
#include "UTIL/OCG/OCG.h"
#include "UTIL/OCG/OCG_extern.h"
#ifndef USER_MODE
#include "PHY/defs.h"

#    define FIFO_PRINTF_MAX_STRING_SIZE   1000
#    define FIFO_PRINTF_NO              62
#    define FIFO_PRINTF_SIZE            65536

#endif

// made static and not local to logRecord() for performance reasons
static char g_buff_info [MAX_LOG_TOTAL];
static char g_buff_infos[MAX_LOG_TOTAL];
static char g_buff_tmp  [MAX_LOG_ITEM];
static char g_buff_debug[MAX_LOG_ITEM];


//static char copyright_string[] __attribute__ ((unused)) =
//  "Copyright (c) The www.openairinterface.org  2009, navid nikaein (navid.nikaein@eurecom.fr) All rights reserved.";

//static const char BUILD_VERSION[] = "v0.1";
//static const char BUILD_DATE[] = "2011-01-15 16:42:14";
//static const char BUILD_HOST[] = "LINUX";
//static const char BUILD_TARGET[] = "OAI";
//#define debug_msg if (((mac_xface->frame%100) == 0) || (mac_xface->frame < 20)) msg

static int fd;

void logInit (int g_log_level) {
  
#ifdef USER_MODE
  g_log = calloc(1, sizeof(log_t));
  memset(g_log, 0, sizeof(log_t));
#else
  g_log = kmalloc(sizeof(log_t),GFP_KERNEL);
#endif
  
    g_log->log_component[LOG].name = "LOG";
    g_log->log_component[LOG].level = LOG_INFO;
    g_log->log_component[LOG].flag =  LOG_MED;
    g_log->log_component[LOG].interval =  1; // in terms of ms or num frames

    g_log->log_component[PHY].name = "PHY";
    g_log->log_component[PHY].level = LOG_INFO;
    g_log->log_component[PHY].flag =  LOG_MED;
    g_log->log_component[PHY].interval =  1;
 
    g_log->log_component[MAC].name = "MAC";
    g_log->log_component[MAC].level = LOG_INFO;
    g_log->log_component[MAC].flag =  LOG_MED;
    g_log->log_component[MAC].interval =  1;
 
    g_log->log_component[OPT].name = "OPT";
    g_log->log_component[OPT].level = LOG_INFO;
    g_log->log_component[OPT].flag = LOG_MED;
    g_log->log_component[OPT].interval =  1;

    g_log->log_component[RLC].name = "RLC";
    g_log->log_component[RLC].level = LOG_INFO;
    g_log->log_component[RLC].flag = LOG_MED;
    g_log->log_component[RLC].interval =  1;

    g_log->log_component[EMU].name = "EMU";
    g_log->log_component[EMU].level = LOG_INFO;
    g_log->log_component[EMU].flag =  LOG_MED; 
    g_log->log_component[EMU].interval =  1;

    g_log->log_component[OMG].name = "OMG";
    g_log->log_component[OMG].level = LOG_INFO;
    g_log->log_component[OMG].flag =  LOG_MED;
    g_log->log_component[OMG].interval =  1;
     
    g_log->log_component[OCG].name = "OCG";
    g_log->log_component[OCG].level = LOG_INFO;
    g_log->log_component[OCG].flag =  LOG_MED;
    g_log->log_component[OCG].interval =  1;

    g_log->log_component[PERF].name = "PERF";
    g_log->log_component[PERF].level = LOG_INFO;
    g_log->log_component[PERF].flag =  LOG_MED;
    g_log->log_component[PERF].interval =  1;

    g_log->log_component[RB].name = "RB";
    g_log->log_component[RB].level = LOG_INFO;
    g_log->log_component[RB].flag =  LOG_MED;
    g_log->log_component[RB].interval =  1;

 
    g_log->level2string[LOG_EMERG]         = "G"; //EMERG
    g_log->level2string[LOG_ALERT]         = "A"; // ALERT
    g_log->level2string[LOG_CRIT]          = "C"; // CRITIC
    g_log->level2string[LOG_ERR]           = "E"; // ERROR
    g_log->level2string[LOG_WARNING]       = "W"; // WARNING
    g_log->level2string[LOG_NOTICE]        = "N"; // NOTICE
    g_log->level2string[LOG_INFO]          = "I"; //INFO
    g_log->level2string[LOG_DEBUG]         = "D"; // DEBUG
    g_log->level2string[LOG_TRACE]         = "T"; // TRACE

    g_log->onlinelog = 1; //online log file
    g_log->syslog = 0; 
    g_log->filelog   = 0;
    g_log->level  = g_log_level;
    g_log->flag   = LOG_MED;
 
#ifdef USER_MODE  
  g_log->config.remote_ip      = 0;
  g_log->config.remote_level   = LOG_EMERG;
  g_log->config.facility       = LOG_LOCAL7;
  g_log->config.audit_ip       = 0;
  g_log->config.audit_facility = LOG_LOCAL6;
  g_log->config.format         = 0x00; // online debug inactive
  
  g_log->filelog_name = "/tmp/openair.log";
 
  if (g_log->syslog) {
    openlog(g_log->log_component[LOG].name, LOG_PID, g_log->config.facility);
  } 
  if (g_log->filelog) {
    fd = open(g_log->filelog_name, O_WRONLY | O_CREAT | O_APPEND, 0666);
  }

#else
  g_log->syslog = 0; 
  g_log->filelog   = 0;
  printk ("[OPENAIR2] LOG INIT\n");
  rtf_create (FIFO_PRINTF_NO, FIFO_PRINTF_SIZE);
#endif
  


}

//inline 
void logRecord( const char *file, const char *func,
		int line,  int comp, int level, 
		char *format, ...) {
   
  int len;
  va_list args;
  log_component_t *c;
  
  g_buff_infos[0] = '\0';
  c = &g_log->log_component[comp];
  
  // only log messages which are enabled and are below the global log level and component's level threshold
  if ((c->level > g_log->level) || (level > c->level)|| (c->flag == LOG_NONE) || 
      (((oai_emulation.info.frame % c->interval) == 0) && (oai_emulation.info.frame > oai_emulation.info.nb_ue_local * 10))) { 
    return;
  }
  // adjust syslog level for TRACE messages
  if (g_log->syslog) {
    if (g_log->level > LOG_DEBUG) {
      g_log->level = LOG_DEBUG;
    }  
  }
  
  va_start(args, format);
  len=vsnprintf(g_buff_info, MAX_LOG_TOTAL, format, args);
  va_end(args);


  if ( g_log->flag & FLAG_COLOR )  {
    len+=snprintf(g_buff_tmp, MAX_LOG_ITEM, "%s",
	     log_level_highlight_start[g_log->level]);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
  
  if ( g_log->flag & FLAG_COMP ){
    len+=snprintf(g_buff_tmp, MAX_LOG_ITEM, "[%s]",
	     g_log->log_component[comp].name);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
  
  if ( g_log->flag & FLAG_LEVEL ){
    len+=snprintf(g_buff_tmp, MAX_LOG_ITEM, "[%s]",
	     g_log->level2string[level]);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
    
  if (  g_log->flag & FLAG_FUNCT )  {
    len+=snprintf(g_buff_tmp, MAX_LOG_ITEM, "[%s] ",
	     func);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
  
  if (  g_log->flag & FLAG_FILE_LINE )  {
    len+=snprintf(g_buff_tmp, MAX_LOG_ITEM, "[%s:%d]",
		  file,line);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
  
  if (  g_log->flag & FLAG_COLOR )  {
    len+=snprintf(g_buff_tmp, MAX_LOG_ITEM, "%s", 
	     log_level_highlight_end[g_log->level]);
    strncat(g_buff_infos, g_buff_tmp, MAX_LOG_TOTAL);
  }
 
  strncat(g_buff_infos, g_buff_info, MAX_LOG_TOTAL);
  //  strncat(g_buff_infos, "\n", MAX_LOG_TOTAL);

#ifdef USER_MODE
  // OAI printf compatibility 
  if (g_log->onlinelog == 1) 
    printf("%s",g_buff_infos);

  if (g_log->syslog) {
    syslog(g_log->level, g_buff_infos);
  } 
  if (g_log->filelog) {
    write(fd, g_buff_infos, strlen(g_buff_infos));
  }
#else
  if (len > MAX_LOG_TOTAL) {
    rt_printk ("[OPENAIR] FIFO_PRINTF WROTE OUTSIDE ITS MEMORY BOUNDARY : ERRORS WILL OCCUR\n");
  }
  if (len <= 0) {
    return ;
  }
  rtf_put (FIFO_PRINTF_NO, g_buff_infos, len);
#endif

}

int  set_comp_log(int component, int level, int flag, int interval) {
  
  if ((component >=MIN_LOG_COMPONENTS) && (component < MAX_LOG_COMPONENTS)){
    if ((flag == LOG_NONE) || (flag == LOG_LOW) || (flag == LOG_MED) || (flag == LOG_FULL)) {
      g_log->log_component[component].flag = flag; 
    }
    if ((level >=LOG_TRACE) && (component <= LOG_EMERG)){
      g_log->log_component[component].level = level;
    }
   if ((interval > 0) && (interval <= 0xFFFF)){
      g_log->log_component[component].interval = interval;
   }
   return 0;
  }
  else
    return -1;
}

void set_glog(int level, int flag) {
  g_log->level = level;
  g_log->flag = flag;
}
void set_glog_syslog(int enable) {
  g_log->syslog = enable;
}
void set_glog_onlinelog(int enable) {
  g_log->onlinelog = enable;
}
void set_glog_filelog(int enable) {
  g_log->filelog = enable;
}


/*
 * for the two functions below, the passed array must have a final entry
 * with string value NULL
 */
/* map a string to an int. Takes a mapping array and a string as arg */
int map_str_to_int(mapping *map, const char *str)
{
    while (1) {
        if (map->name == NULL) {
            return(-1);
        }
        if (!strcmp(map->name, str)) {
            return(map->value);
        }
        map++;
    }
}

/* map an int to a string. Takes a mapping array and a value */
char *map_int_to_str(mapping *map, int val)
{
    while (1) {
        if (map->name == NULL) {
            return NULL;
        }
        if (map->value == val) {
            return map->name;
        }
        map++;
    }
}

void logClean (void)
{
#ifndef USER_MODE
  rtf_destroy (FIFO_PRINTF_NO);
#else
if (g_log->syslog) {
  closelog();
 } 
 if (g_log->filelog) {
  close(fd);
 }
#endif

}


#ifdef LOG_TEST

int
main(int argc, char *argv[]) {

  logInit();

  //set_log_syslog(1);
  test_log();
  
  return 1;
}

int test_log(){

  LOG_ENTER(MAC); // because the default level is DEBUG
  LOG_I(LOG, "1 Starting OAI logs version %s Build date: %s on %s\n", 
	       BUILD_VERSION, BUILD_DATE, BUILD_HOST);  
  LOG_D(MAC, "1 debug  MAC \n");
  LOG_N(MAC, "1 notice MAC \n");
  LOG_W(MAC, "1 warning MAC \n");
 
  set_comp_log(LOG, LOG_INFO, FLAG_ONLINE);
  set_comp_log(MAC, LOG_WARNING, 0);
  
  LOG_ENTER(MAC);
  LOG_I(LOG, "2 Starting OAI logs version %s Build date: %s on %s\n", 
	       BUILD_VERSION, BUILD_DATE, BUILD_HOST);  
  LOG_E(MAC, "2 emerge MAC\n");
  LOG_D(MAC, "2 debug  MAC \n");
  LOG_N(MAC, "2 notice MAC \n");
  LOG_W(MAC, "2 warning MAC \n");
  LOG_I(MAC, "2 info MAC \n");
  
  
  set_comp_log(MAC, LOG_NOTICE, 1);
  
  LOG_ENTER(MAC);
  LOG_I(LOG, "3 Starting OAI logs version %s Build date: %s on %s\n", 
	       BUILD_VERSION, BUILD_DATE, BUILD_HOST);  
  LOG_D(MAC, "3 debug  MAC \n");
  LOG_N(MAC, "3 notice MAC \n");
  LOG_W(MAC, "3 warning MAC \n");
  LOG_I(MAC, "3 info MAC \n");
  
  set_comp_log(MAC, LOG_DEBUG,1);
  set_comp_log(LOG, LOG_DEBUG,1);
 
  LOG_ENTER(MAC);
  LOG_I(LOG, "4 Starting OAI logs version %s Build date: %s on %s\n", 
	       BUILD_VERSION, BUILD_DATE, BUILD_HOST);  
  LOG_D(MAC, "4 debug  MAC \n");
  LOG_N(MAC, "4 notice MAC \n");
  LOG_W(MAC, "4 warning MAC \n");
  LOG_I(MAC, "4 info MAC \n");

 
  set_comp_log(MAC, LOG_DEBUG,0);
  set_comp_log(LOG, LOG_DEBUG,0);
 
  LOG_I(LOG, "5 Starting OAI logs version %s Build date: %s on %s\n", 
	       BUILD_VERSION, BUILD_DATE, BUILD_HOST);  
  LOG_D(MAC, "5 debug  MAC \n");
  LOG_N(MAC, "5 notice MAC \n");
  LOG_W(MAC, "5 warning MAC \n");
  LOG_I(MAC, "5 info MAC \n");
  
 
  set_comp_log(MAC, LOG_TRACE,0X07F);
  set_comp_log(LOG, LOG_TRACE,0X07F);
  
  LOG_ENTER(MAC);
  LOG_I(LOG, "6 Starting OAI logs version %s Build date: %s on %s\n", 
	BUILD_VERSION, BUILD_DATE, BUILD_HOST);  
  LOG_D(MAC, "6 debug  MAC \n");
  LOG_N(MAC, "6 notice MAC \n");
  LOG_W(MAC, "6 warning MAC \n");
  LOG_I(MAC, "6 info MAC \n");
  LOG_EXIT(MAC);

}
#endif
