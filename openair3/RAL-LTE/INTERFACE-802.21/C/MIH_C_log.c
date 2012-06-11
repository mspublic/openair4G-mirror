/***************************************************************************
                          MIH_C_log.c  -  description
                             -------------------
    copyright            : (C) 2007-2012 by Eurecom
 ***************************************************************************/
#define MIH_C_INTERFACE
#define MIH_C_LOG_C
//-----------------------------------------------------------------------------
#include "MIH_C_log.h"
//-----------------------------------------------------------------------------
// Initialize logging system
int MIH_C_log_init(unsigned int log_outputP) {
//-----------------------------------------------------------------------------
    g_mih_c_log_output = log_outputP;


    g_log_level2string[LOG_EMERG]         = "LOG_EMERG";
    g_log_level2string[LOG_ALERT]         = "LOG_ALERT";
    g_log_level2string[LOG_CRIT]          = "CRIT     ";
    g_log_level2string[LOG_ERR]           = "ERR      ";
    g_log_level2string[LOG_WARNING]       = "WARNING  ";
    g_log_level2string[LOG_NOTICE]        = "NOTICE   ";
    g_log_level2string[LOG_INFO]          = "INFO     ";
    g_log_level2string[LOG_DEBUG]         = "DEBUG    ";



    switch (g_mih_c_log_output){
        case LOG_TO_CONSOLE:
            printf("***** mRAL V%s logging \n\n",MIH_C_VERSION);
            break;
        case LOG_TO_FILE:
            g_mih_c_log_file = fopen(MIH_C_LOGFILE_NAME,"w"); //start over new file
            if (g_mih_c_log_file == NULL){
                perror ("MIH_C_log_init - error opening file");
                exit(1);
            }
            fprintf(g_mih_c_log_file, "***** V%s starting logging \n\n",MIH_C_VERSION);
            fflush(g_mih_c_log_file);
            break;
        case LOG_TO_SYSTEM:
            openlog(MIH_C_SYSLOG_NAME, LOG_PID, LOG_LOCAL7);
            syslog(LOG_NOTICE,  "***** V%s starting logging \n\n",MIH_C_VERSION);
            break;
        default:
            printf("MIH_C_log_init: log_outputP error %d", log_outputP);
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Log messages according to user settings
int MIH_C_log_record(int levelP, const char * log_msgP, ...) {
//-----------------------------------------------------------------------------
    struct timespec time_spec;
    unsigned int time_now_micros;
    unsigned int time_now_s;
    va_list log_ap;

    switch (g_mih_c_log_output){
        case LOG_TO_CONSOLE:

            clock_gettime (CLOCK_REALTIME, &time_spec);
            time_now_s      = (unsigned int) time_spec.tv_sec;
            time_now_micros = (unsigned int) time_spec.tv_nsec/1000;
            printf("[%06d:%06d][%s][%s]", time_now_s, time_now_micros, MIH_C_SYSLOG_NAME, g_log_level2string[levelP]);
            va_start(log_ap, log_msgP);
            vprintf(log_msgP, log_ap);
            va_end(log_ap);
            fflush(stdout);
            fflush(stderr);
            break;
        case LOG_TO_FILE:
            clock_gettime (CLOCK_REALTIME, &time_spec);
            time_now_s      = (unsigned int) time_spec.tv_sec;
            time_now_micros = (unsigned int) time_spec.tv_nsec/1000;
            fprintf(g_mih_c_log_file, "[%06d:%06d][%s][%s] ", time_now_s, time_now_micros, MIH_C_SYSLOG_NAME, g_log_level2string[levelP]);
            va_start(log_ap, log_msgP);
            vfprintf(g_mih_c_log_file,log_msgP, log_ap);
            va_end(log_ap);
            fflush(g_mih_c_log_file);
            break;
        case LOG_TO_SYSTEM:
            va_start(log_ap, log_msgP);
            syslog(levelP, log_msgP, log_ap);
            va_end(log_ap);
            break;
        default:
            printf("MIH_C_log_record: level error %d", levelP);
  }
  return 0;
}
//-----------------------------------------------------------------------------
// Close logging system
int MIH_C_log_exit(void) {
//-----------------------------------------------------------------------------
    switch (g_mih_c_log_output){
        case LOG_TO_CONSOLE:
            printf("***** stopping logging \n\n");
            break;
        case LOG_TO_FILE:
            fprintf(g_mih_c_log_file, "***** stopping logging \n\n");
            fflush(g_mih_c_log_file);
            fclose(g_mih_c_log_file);
            break;
        case LOG_TO_SYSTEM:
            syslog(LOG_NOTICE,  "***** stopping logging \n\n");
            closelog();
            break;
        default:
            printf("MIH_C_log_exit: output unrecognized error %d", g_mih_c_log_output);
    }
    return 0;
}


