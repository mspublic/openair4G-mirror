/*________________________mac_sim.c________________________

  Authors : Hicham Anouar, Raymond Knopp
  Company : EURECOM
  Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
  ________________________________________________________________*/



//#include "openair_proto.h"
//#include "openair_vars.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>

#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/emul_device.h"
#include "COMMON/platform_constants.h"
#include "SIMULATION/config_proto.h"
#include "SIMULATION/simulation_defs.h"
#include "SIMULATION/config_vars.h"
#include "SIMULATION/phy_vars.h"

EMULATION_VARS Emul_vars_mem,*Emul_vars;
PHY_CONFIG PHY_config_mem;

int main (int argc, char **argv) {
  //-----------------------------------------------------------------------------

  int openair_fd;

  unsigned int action;

  char device_name[16];
  unsigned char temp[4];
  unsigned int tmp;
  int result=-1;
  
  unsigned int Topo_info[100];//H.A
  
  FILE *topology,*CM;
  char topology_file[256];

  
  
  if (argc < 2) {
    printf("[openair][INFO] Usage %s  action (0-14) params . . .  \n" , argv[0]);
    printf("[openair][INFO] ACTION DESCRIPTIONS\n");
    printf("[openair][INFO] Action 0  : Dump Emulation Configurations to Kernel\n");
    printf("[openair][INFO] Action 1  : Start Emulation\n");
    printf("[openair][INFO] Action 2  : Suspend Emulation\n");
    exit (-1);
  }
  
  action = atoi(argv[1]);
  
  if ((action == 0) && (argc < 3)) {
    printf("[openair][INFO] Provide Machine ID ...  \n");
    exit(-1);
  }
  
  
  if (action == 0) {// configure 
    printf("[openair][INFO][START] Action              is : configuration\n");
    Emul_vars = (EMULATION_VARS *)&Emul_vars_mem;
    printf("Emul_vars = %p\n",Emul_vars);
    if (argc<4) {
      printf("[openair][INFO][START] Provide a topology file ...\n");
      exit(-1);
    }
    Master_id=atoi(argv[2]);
    strcpy(topology_file,argv[3]);
    printf("[OPENAIR][INFO]CONFIG: Master_id %d, Topo file %s\n",Master_id,topology_file);

  }
  else if (action == 1) // start clusterhead
    printf("[openair][INFO][START] Action              is : start emulation\n");
  else if (action == 2) // start terminode
    printf("[openair][INFO][START] Action              is : suspend emulation\n");

  
  
  printf("[openair][INFO][START] Opening /dev/openair0\n");
  if ((openair_fd = open("/dev/openair0", O_RDWR,0)) <0) {
    fprintf(stderr,"Error %d opening /dev/openair0\n",openair_fd);
    exit(-1);
  }
  
  if (action == 0) {
    if((config = fopen("../../openair_config.cfg","r")) == NULL) // this can be configured
      {
	printf("[openair][CONFIG][INFO] openair configuration file <openair_config.cfg> could not be found!");
	exit(0);
      }
    
    
    if ((scenario= fopen("../../openair_scenario.scn","r")) ==NULL)
      {
	printf("\n[openair][CONFIG][INFO] openair scenario file <openair_scenario.scn> could not be found!");
	exit(0);
      }
    
    if ((topology= fopen(topology_file,"r")) ==NULL)
      {
	printf("\n[openair][CONFIG][INFO] The openair topology file <%s> could not be found!\n",topology_file);
	exit(0);
      }
    if ((CM= fopen("../../covariance_matrix.dat","r")) ==NULL)
      {
	printf("\n[openair][CONFIG][INFO] The openair Covariance matrix file <Covariance_matrix.tex> could not be found!\n");
	exit(0);
      }  
  }
  
  
  printf("Running action %d\n",action);
  switch (action) {
    
  case 0 :
    PHY_config = (PHY_CONFIG *)&PHY_config_mem;
    Master_id=atoi(argv[2]);
    printf("Configuring Master %d\n",Master_id);

    config_topology(topology);
    radio_emulation_load_KH(CM);
    printf("Emul_vars = %p\n",Emul_vars);

    reconfigure_MACPHY(scenario);
    printf("reconfigure_MACPHY() done.\n");fflush(stdout);
    
    
    result=ioctl(openair_fd, EMUL_DUMP_CONFIG,(char *)PHY_config);
    if (result == 0) {
      printf ("[openair][CONFIG][INFO] loading openair configuration in kernel space\n");
    } else {
      printf ("[openair][START][INFO] loading openair configuration in kernel space failed \n");
    }
    
#undef NB_INST

          printf("[openair][START][INFO] Sending topology for %d instances (Emul_vars %p,%d)\n",Emul_vars->NB_INST,Emul_vars,sizeof(EMULATION_VARS));
    result=ioctl(openair_fd, EMUL_GET_TOPOLOGY,(char *)Emul_vars);
    if (result == 0) {
      printf ("[openair][CONFIG][INFO] loading openair configuration in kernel space\n");
    } else {
      printf ("[openair][START][INFO] loading openair configuration in kernel space failed \n");
    }
    
    
    
    break;
    
  case 1 :
    printf("[openair][START][INFO] Starting emulation\n");
    
    result=ioctl(openair_fd,EMUL_START, NULL);
    
    break;
  case 2 :
    printf("[openair][START][INFO] Suspending emulation \n");
    result=ioctl(openair_fd,EMUL_STOP,NULL);
    if (result == 0) {
      printf ("[openair][START][INFO] emulation suspended\n");
    } else {
      printf ("[openair][START][INFO] failed \n");
    }
    
    break;
    
    
  }
}


