#define NB_ANTENNAS_TX 2
#define NB_ANTENNAS_RX 2
#define USER_MODE
#define EMOS
#define OPENAIR_LTE

//#include <gps.h>
#include "PHY/defs.h"

int main(void)
{
  printf("fifo_dump_emos_UE_size = %d\n",sizeof(fifo_dump_emos_UE));
  //printf("gps_fix_t_size = %x\n",sizeof(struct gps_fix_t));

  return(0);
}
