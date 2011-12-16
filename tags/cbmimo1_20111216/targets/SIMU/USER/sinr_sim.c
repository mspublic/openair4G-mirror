#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <cblas.h>

#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/extern.h"
#include "oaisim_config.h"

#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log_if.h"
#include "RRC/LITE/extern.h"
#include "PHY_INTERFACE/extern.h"
#include "UTIL/OCG/OCG.h"
#include "UTIL/OMG/omg.h"
#include "UTIL/OPT/opt.h" // to test OPT
#endif

#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "oaisim.h"

#define PI 3.1416
#define Am 20
#define MCS_COUNT 23
#define MCL (-70) /*minimum coupling loss (MCL) in dB*/
//double sinr[NUMBER_OF_eNB_MAX][2*25];
extern double sinr_bler_map[MCS_COUNT][2][9];


// Extract the positions of UE and ENB from the mobility model 

void extract_position (Node_list input_node_list, node_desc_t **node_data, int nb_nodes) {    
    
  int i;
  for (i=0;i<nb_nodes;i++) {
    if ((input_node_list != NULL) &&  (node_data[i] != NULL)) {
      node_data[i]->x = input_node_list->node->X_pos;
      node_data[i]->y = input_node_list->node->Y_pos;
      input_node_list = input_node_list->next;
    }
    else {
      printf("extract_position: Null pointer!!!\n");
      exit(-1);
    }
  }
}

void init_ue(node_desc_t  *ue_data, UE_Antenna ue_ant) {//changed from node_struct

  ue_data->n_sectors = 1;
  ue_data->phi_rad = 2 * PI;
  ue_data->ant_gain_dBi = ue_ant.antenna_gain_dBi;
  ue_data->tx_power_dBm = ue_ant.tx_power_dBm;
  ue_data->rx_noise_level = ue_ant.rx_noise_level_dB; //value in db

}

void init_enb(node_desc_t  *enb_data, eNB_Antenna enb_ant) {//changed from node_struct

  int i;
  double sect_angle[3]={0,2*PI/3,4*PI/3};

  enb_data->n_sectors = enb_ant.number_of_sectors;
  for (i=0;i<3;i++) 
    enb_data->alpha_rad[i] = sect_angle[i]; //enb_ant.alpha_rad[i]; 
  enb_data->phi_rad = enb_ant.beam_width_dB;
  enb_data->ant_gain_dBi = enb_ant.antenna_gain_dBi;
  enb_data->tx_power_dBm = enb_ant.tx_power_dBm;
  enb_data->rx_noise_level = enb_ant.rx_noise_level_dB; 

}



void calc_path_loss(node_desc_t* enb_data, node_desc_t* ue_data, channel_desc_t *ch_desc, Environment_System_Config env_desc, double Shad_Fad) {  

  double dist; 
  double path_loss;
  double gain_max;
  double gain_sec[3];
  double alpha, theta;
  
  int count;

 
  dist = sqrt(pow((enb_data->x - ue_data->x), 2) + pow((enb_data->y - ue_data->y), 2));
  
  path_loss = -(env_desc.fading.free_space_model_parameters.pathloss_0_dB + 
		10*env_desc.fading.free_space_model_parameters.pathloss_exponent * log10(dist/1000)); 
  //printf("dist %f, Path Loss %f\n",dist,ch_desc->path_loss_dB);

  /* Calculating the angle in the range -pi to pi from the slope */
  alpha = atan2((ue_data->x - enb_data->x), (ue_data->y - enb_data->y));
  if (alpha < 0)
    alpha += 2*PI; 
  //printf("angle in radians is %lf\n", ue_data[UE_id]->alpha_rad[eNB_id]);
  ch_desc->aoa = alpha;
      
  gain_max = -1000;
  for(count = 0; count < enb_data->n_sectors; count++) {
    theta = enb_data->alpha_rad[count] - alpha;
    /* gain = -min(Am , 12 * (theta/theta_3dB)^2) */
    gain_sec[count] = -(Am < (12 * pow((theta/enb_data->phi_rad),2)) ? Am : (12 * pow((theta/enb_data->phi_rad),2)));
    if (gain_sec[count]>gain_max)
      gain_max = gain_sec[count];
  }

  path_loss += enb_data->ant_gain_dBi + gain_max + ue_data->ant_gain_dBi;
  path_loss += Shad_Fad;

  ch_desc->path_loss_dB = MCL < path_loss ?  MCL : path_loss;
  //printf("x_coordinate\t%f\t,y_coordinate\t%f\t, path_loss %f\n",ue_data->x,ue_data->y,ch_desc->path_loss_dB);
}





void init_snr(channel_desc_t* eNB2UE, node_desc_t *enb_data, node_desc_t *ue_data, double* sinr_dB, double* N0) {

  int return_value;
  u16 nb_rb = 25; //No. of resource blocks
  double thermal_noise;
  int count;
  int aarx;
      
  /* Thermal noise is calculated using 10log10(K*T*B) K = Boltzmann's constant T = room temperature B = bandwidth */
  thermal_noise = -174 + 10*log10(eNB2UE->BW*1e6); //value in dBm 

  //for (aarx=0; aarx<eNB2UE->nb_rx; aarx++)
    *N0 = thermal_noise + ue_data->rx_noise_level;//? all the element have the same noise level?????
      
  printf("[CHANNEL_SIM] path loss %lf, noise %lf, signal %lf, snr %lf\n", 
	 eNB2UE->path_loss_dB, 
	 thermal_noise + ue_data->rx_noise_level,
	 enb_data->tx_power_dBm + eNB2UE->path_loss_dB,
	 enb_data->tx_power_dBm + eNB2UE->path_loss_dB - (thermal_noise + ue_data->rx_noise_level));

    //printf ("coupling factor is %lf\n", coupling); 
    for (count = 0; count < (2 * nb_rb); count++) {
      sinr_dB[count] = enb_data->tx_power_dBm 
	+ eNB2UE->path_loss_dB
	- (thermal_noise + ue_data->rx_noise_level)  
	+ 10 * log10 (pow(eNB2UE->chF[0][count].x, 2) 
		      + pow(eNB2UE->chF[0][count].y, 2));
      //printf("Dl_link SNR for res. block %d is %lf\n", count, sinr[eNB_id][count]);
    } 
}//function ends





void calculate_sinr(channel_desc_t* eNB2UE, node_desc_t *enb_data, node_desc_t *ue_data, double *sinr_dB) {

  double sir, thermal_noise;
  short nb_rb = 25; //No. of resource blocks
  short count;

  /* Thermal noise is calculated using 10log10(K*T*B) K = Boltzmann's constant T = room temperature B = bandwidth */
  thermal_noise = -174 + 10*log10(eNB2UE->BW*1e6); //value in dBm 

  for (count = 0; count < 2 * nb_rb; count++) {
    sir = enb_data->tx_power_dBm 
      + eNB2UE->path_loss_dB
      - (thermal_noise + ue_data->rx_noise_level)  
      + 10 * log10 (pow(eNB2UE->chF[0][count].x, 2) 
		    + pow(eNB2UE->chF[0][count].y, 2));
    if (sir > 0)
      sinr_dB[count] -= sir;
    //printf("*****sinr% lf \n",sinr_dB[count]);
  }
}

void get_beta_map() {
  char *file_path = NULL;
  /*
  char *file_name[] = {"bler_1.csv", "bler_2.csv", "bler_3.csv", "bler_4.csv", "bler_5.csv", "bler_6.csv", "bler_7.csv", "bler_8.csv",
                    "bler_9.csv", "bler_10.csv", "bler_11.csv", "bler_12.csv", "bler_13.csv", "bler_14.csv", "bler_15.csv", "bler_16.csv",
                    "bler_17.csv", "bler_18.csv", "bler_19.csv", "bler_20.csv", "bler_21.csv", "bler_22.csv"};
  */
  int table_len = 0;
  int mcs = 0;
  char *sinr_bler;
  char buffer[100];
  FILE *fp;

  /*
  file_path_ptr = strcat(file_path_ptr,getenv("OPENAIR1_DIR"));
  file_path_ptr = strcat(file_path_ptr,"/SIMULATION/LTE_PHY/Abstraction");
  */
  file_path = (char*) malloc(256);

  for (mcs = 0; mcs <= 4; mcs++) {
    for (table_len = 0; table_len < 9; table_len++) {
      sinr_bler_map[mcs][0][table_len] = 0.0;
      sinr_bler_map[mcs][1][table_len] = 0.0;
    }
  }

  for (mcs = 5; mcs < MCS_COUNT; mcs++) {
    sprintf(file_path,"%s/SIMULATION/LTE_PHY/Abstraction/bler_%d.csv",getenv("OPENAIR1_DIR"),mcs);
    fp = fopen(file_path,"r");
    if (fp == NULL) {
      printf("ERROR: Unable to open the file %s\n", file_path);
      exit(-1);
    }
    else {
      fgets(buffer, 100, fp);
      table_len=0;
      while (!feof(fp)) {
        sinr_bler = strtok(buffer, ";");
        sinr_bler_map[mcs][0][table_len] = atof(sinr_bler);
        sinr_bler = strtok(NULL,";");
        sinr_bler_map[mcs][1][table_len] = atof(sinr_bler);
        table_len++;
        fgets(buffer, 100, fp);
      }
      fclose(fp);
    }
    printf("\n table for mcs %d\n",mcs);
    for (table_len = 0; table_len < 9; table_len++)
      printf("%lf  %lf \n ",sinr_bler_map[mcs][0][table_len],sinr_bler_map[mcs][1][table_len]);
  }
  free(file_path);
}


