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
* \warning This component can be run only in user-space
* @ingroup routing

*/

//#define LOG_TEST 1

#define COMPONENT_LOG
#define COMPONENT_LOG_IF



//static unsigned char       fifo_print_buffer[FIFO_PRINTF_MAX_STRING_SIZE];

#include "log.h"

#ifndef USER_MODE
#include "PHY/defs.h"

#    define FIFO_PRINTF_MAX_STRING_SIZE   500
#    define FIFO_PRINTF_NO              62
#    define FIFO_PRINTF_SIZE            65536

#endif

// made static and not local to logRecord() for performance reasons
static char g_buff_info [MAX_LOG_TOTAL];
static char g_buff_infos[MAX_LOG_TOTAL];
static char g_buff_tmp  [MAX_LOG_ITEM];
static char g_buff_debug[MAX_LOG_ITEM];

static int thread_safe_debug_count = 0;

//static char copyright_string[] __attribute__ ((unused)) =
//  "Copyright (c) The www.openairinterface.org  2009, navid nikaein (navid.nikaein@eurecom.fr) All rights reserved.";

//static const char BUILD_VERSION[] = "v0.1";
//static const char BUILD_DATE[] = "2011-01-15 16:42:14";
//static const char BUILD_HOST[] = "LINUX";
//static const char BUILD_TARGET[] = "OAI";


void logInit ()
{
  //#ifdef USER_MODE
  //printf("[LOG] logInit(%p)\n", this);
#ifdef USER_MODE
  g_log = calloc(1, sizeof(log_t));
#else
  g_log = kmalloc(sizeof(log_t),GFP_KERNEL);
#endif

  memset(g_log, 0, sizeof(log_t));

  g_log->log_component[LOG].name = "LOG";
  g_log->log_component[LOG].level = LOG_TRACE;
  g_log->log_component[LOG].flag = LOG_MED;
  
  g_log->log_component[MAC].name = "MAC";
  g_log->log_component[MAC].level = LOG_INFO;
  g_log->log_component[MAC].flag = LOG_MED;
  
  g_log->log_component[OCG].name = "OCG";
  g_log->log_component[OCG].level = LOG_INFO;
  g_log->log_component[OCG].flag = LOG_MED;

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
  g_log->flag    = LOG_MED;
#ifdef USER_MODE  
  g_log->config.remote_ip      = 0;
  g_log->config.remote_level   = LOG_EMERG;
  g_log->config.facility       = LOG_LOCAL7;
  g_log->config.audit_ip       = 0;
  g_log->config.audit_facility = LOG_LOCAL6;
  g_log->config.format         = 0x00; // online debug inactive
  
  g_log->log_file_name = "openair.log";
#else
  printk ("[OPENAIR2] TRACE INIT\n");
  rtf_create (FIFO_PRINTF_NO, FIFO_PRINTF_SIZE);
#endif
  
}

void logRecord( const char *file, const char *func,
		int line,  int comp, int level, 
		char *format, ...) {
   
  int len;
  va_list args;
  log_component_t *c;
  
  //#ifdef USER_MODE
  //thread_safe_debug_count++;
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
  //#endif 

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
  strncat(g_buff_infos, "\n", MAX_LOG_TOTAL);

#ifdef USER_MODE
  // OAI printf compatibility 
  if (g_log->flag & FLAG_ONLINE || c->flag & FLAG_ONLINE) 
    printf("%s",g_buff_infos);

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
#else
  if (len > MAX_LOG_TOTAL) {
    rt_printk ("[OPENAIR] FIFO_PRINTF WROTE OUTSIDE ITS MEMORY BOUNDARY : ERRORS WILL OCCUR\n");
  }
  if (len <= 0) {
    return 0;
  }
  rtf_put (FIFO_PRINTF_NO, g_buff_infos, len);
#endif

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

void logClean (void)
{
#ifndef USER_MODE
  rtf_destroy (FIFO_PRINTF_NO);
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
