#include <unistd.h>
#ifdef OMP
#include <omp.h>
#endif

typedef struct {

  long long in;
  long long diff;
  long long max;
  int trials;
} time_stats_t;

static inline void start_meas(time_stats_t *ts) __attribute__((always_inline));
static inline void stop_meas(time_stats_t *ts) __attribute__((always_inline));

void print_meas(time_stats_t *ts, const char* name);

#if defined(__i386__)
static inline unsigned long long rdtsc_oai(void) __attribute__((always_inline));
static inline unsigned long long rdtsc_oai(void) {
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}
#elif defined(__x86_64__)
static inline unsigned long long rdtsc_oai() __attribute__((always_inline));
static inline unsigned long long rdtsc_oai() { 
  unsigned long long a, d;
  __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
  return (d<<32) | a;
}
#endif

static inline void start_meas(time_stats_t *ts) {

#ifdef OMP
  int tid;

  tid = omp_get_thread_num();
  if (tid==0)
#endif
    {
      ts->trials++;
      ts->in = rdtsc_oai();
    }
}

static inline void stop_meas(time_stats_t *ts) {

  long long out = rdtsc_oai();

#ifdef OMP
  int tid;
  tid = omp_get_thread_num();
  if (tid==0)
#endif
    {
      ts->diff += (out-ts->in);
      if ((out-ts->in) > ts->max)
	ts->max = out-ts->in;
      
    }
}

static inline void reset_meas(time_stats_t *ts) {

  ts->trials=0;
  ts->diff=0;
  ts->max=0;
}

/*static inline double get_mean_meas_us(time_stats_t *ts, double cpu_freq_GHz) {

  return (double) ts->diff/ts->trials/cpu_freq_GHz/1000.0;

  }*/

static inline double get_cpu_freq_GHz(void) {

  time_stats_t ts;
  reset_meas(&ts);
  start_meas(&ts);
  sleep(1);
  stop_meas(&ts);
  return (double)ts.diff/1000000000;
}
