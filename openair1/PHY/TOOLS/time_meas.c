#include <stdio.h>
#include "time_meas.h"

void print_meas(time_stats_t *ts, const char* name) {

  static double cpu_freq_GHz = 0.0;

  if (cpu_freq_GHz == 0.0)
    cpu_freq_GHz = get_cpu_freq_GHz();
  
  if (ts->trials>0)
    //printf("%20s: total: %10.3f ms, average: %10.3f us (%10d trials)\n", name, ts->diff/cpu_freq_GHz/1000000.0, ts->diff/ts->trials/cpu_freq_GHz/1000.0, ts->trials);
    fprintf(stderr, "%20s; %10.3f; %10.3f; %10d\n", name, ts->diff/cpu_freq_GHz/1000000.0, ts->diff/ts->trials/cpu_freq_GHz/1000.0, ts->trials);

}
