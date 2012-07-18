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
#include "UTIL/LOG/log_extern.h"
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
#define MCS_COUNT 24
#define MCL (-70) /*minimum coupling loss (MCL) in dB*/
//double sinr[NUMBER_OF_eNB_MAX][2*25];
extern double sinr_bler_map[MCS_COUNT][2][16];

extern double MI_map_4qam[3][162];
extern double MI_map_16qam[3][197];
extern double MI_map_64qam[3][227];

// Extract the positions of UE and ENB from the mobility model 

void extract_position (Node_list input_node_list, node_desc_t **node_data, int nb_nodes) {    
    
  int i;
  for (i=0;i<nb_nodes;i++) {
   if ((input_node_list != NULL) &&  (node_data[i] != NULL)) {
      
      node_data[i]->x = input_node_list->node->X_pos;
      // JHNOTE (01/05/2012): had to add this safety check as SUMO might return negative positions at the initialization (node 'just' at the boundaries)
      if (node_data[i]->x <0.0)
        node_data[i]->x = 0.0;
      node_data[i]->y = input_node_list->node->Y_pos;
      if (node_data[i]->y <0.0)
        node_data[i]->y = 0.0;
      LOG_I(OCM, "extract_position: added node_data %d with position X: %f and Y: %f \n", i,input_node_list->node->X_pos, input_node_list->node->Y_pos );
      input_node_list = input_node_list->next;
    }
    else {
      LOG_E(OCM, "extract_position: Null pointer!!!\n");
      //exit(-1);
    }
  }
}

void extract_position_fixed_ue (node_desc_t **node_data, int nb_nodes) {    
    
  int i;
  for (i=0;i<nb_nodes;i++) {
    if (i==0) {
      node_data[i]->x = 1856;
      node_data[i]->y = 1813;
    }
    else {
      node_data[i]->x = 2106;
      node_data[i]->y = 1563;
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
  for (i=0;i<enb_data->n_sectors;i++) 
    enb_data->alpha_rad[i] = sect_angle[i]; //enb_ant.alpha_rad[i]; 
  enb_data->phi_rad = enb_ant.beam_width_dB;
  enb_data->ant_gain_dBi = enb_ant.antenna_gain_dBi;
  enb_data->tx_power_dBm = enb_ant.tx_power_dBm;
  enb_data->rx_noise_level = enb_ant.rx_noise_level_dB; 

}



void calc_path_loss(node_desc_t* enb_data, node_desc_t* ue_data, channel_desc_t *ch_desc, Environment_System_Config env_desc, double **Shad_Fad) {  

  double dist; 
  double path_loss;
  double gain_max;
  double gain_sec[3];
  double alpha, theta;
  
  int count;

 
  dist = sqrt(pow((enb_data->x - ue_data->x), 2) + pow((enb_data->y - ue_data->y), 2));
  
  path_loss = env_desc.fading.free_space_model_parameters.pathloss_0_dB - 
		10*env_desc.fading.free_space_model_parameters.pathloss_exponent * log10(dist/1000); 
  //printf("dist %f, Path loss %f\n",dist,ch_desc->path_loss_dB);

  /* Calculating the angle in the range -pi to pi from the slope */
  alpha = atan2((ue_data->x - enb_data->x), (ue_data->y - enb_data->y));
  if (alpha < 0)
    alpha += 2*PI; 
  //printf("angle in radians is %lf\n", ue_data[UE_id]->alpha_rad[eNB_id]);
  ch_desc->aoa = alpha;
  ch_desc->random_aoa = 0;
      
  if (enb_data->n_sectors==1) //assume omnidirectional antenna
    gain_max = 0;
  else {
    gain_max = -1000;
    for(count = 0; count < enb_data->n_sectors; count++) {
      theta = enb_data->alpha_rad[count] - alpha;
      /* gain = -min(Am , 12 * (theta/theta_3dB)^2) */
      gain_sec[count] = -(Am < (12 * pow((theta/enb_data->phi_rad),2)) ? Am : (12 * pow((theta/enb_data->phi_rad),2)));
      if (gain_sec[count]>gain_max)  //take the sector that we are closest too (or where the gain is maximum)
	gain_max = gain_sec[count];
    }
  }
  path_loss += enb_data->ant_gain_dBi + gain_max + ue_data->ant_gain_dBi;
  if (Shad_Fad!=NULL)
    path_loss += Shad_Fad[(int)ue_data->x][(int)ue_data->y];

  ch_desc->path_loss_dB = MCL < path_loss ?  MCL : path_loss;
  //LOG_D(OCM,"x_coordinate\t%f\t,y_coordinate\t%f\t, path_loss %f\n",ue_data->x,ue_data->y,ch_desc->path_loss_dB);
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
      
    LOG_D(OCM,"Path loss %lf, noise %lf, signal %lf, snr %lf\n", 
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
  int table_len = 0;
  int mcs = 0;
  char *sinr_bler;
  char buffer[1000];
  FILE *fp;

  file_path = (char*) malloc(512);

  for (mcs = 0; mcs < MCS_COUNT; mcs++) {
    sprintf(file_path,"%s/SIMULATION/LTE_PHY/BLER_SIMULATIONS/AWGN/awgn_abst/awgn_snr_bler_mcs%d.csv",getenv("OPENAIR1_DIR"),mcs);
    fp = fopen(file_path,"r");
    if (fp == NULL) {
      LOG_E(OCM,"ERROR: Unable to open the file %s\n", file_path);
      exit(-1);
    }
    else {
      fgets(buffer, 1000, fp);
      table_len=0;
      while (!feof(fp)) {
        sinr_bler = strtok(buffer, ",");
        sinr_bler_map[mcs][0][table_len] = atof(sinr_bler);
        sinr_bler = strtok(NULL,",");
        sinr_bler_map[mcs][1][table_len] = atof(sinr_bler);
        table_len++;
        fgets(buffer, 1000, fp);
      }
      fclose(fp);
    }
    LOG_D(OCM," Print the table for mcs %d\n",mcs);
    for (table_len = 0; table_len < 16; table_len++)
      LOG_D(OCM,"%lf  %lf \n ",sinr_bler_map[mcs][0][table_len],sinr_bler_map[mcs][1][table_len]);
  }
  free(file_path);
}

//this function reads and stores the Mutual information tables for the MIESM abstraction. 
void get_MIESM_param() {
  char *file_path = NULL;
  char buffer[10000];
  FILE *fp;
  int qam[3] = {4,16,64};
  int q,cnt;
  char *result = NULL;
  int table_length=0;
  int table_len;
  file_path = (char*) malloc(512);
  for (q=0;q<3;q++)
    {
      sprintf(file_path,"%s/SIMU/USER/files/MI_%dqam.csv",getenv("OPENAIR_TARGETS"),qam[q]);
      fp = fopen(file_path,"r");
      if (fp == NULL) {
	printf("ERROR: Unable to open the file %s\n", file_path);
	exit(-1);
      }
      else {
	cnt=-1;
	switch(qam[q]) {
	case 4: 	  
	  while (!feof(fp)) {
	    table_length =0;
	    cnt++;
	    fgets(buffer, 10000, fp);
	    result = strtok(buffer, ",");
	    while (result != NULL) {
	      MI_map_4qam[cnt][table_length]= atof(result);
	      result = strtok(NULL, ",");
	      table_length++;
	    }
	  }
       fclose(fp);
       for (table_len = 0; table_len < 162; table_len++)
	 printf("MIESM 4QAM Table: %lf  %lf  %1f\n ",MI_map_4qam[0][table_len],MI_map_4qam[1][table_len], MI_map_4qam[2][table_len]);
       break;
	case 16:
	   while (!feof(fp)) {
	    table_length =0;
	    cnt++;
	    fgets(buffer, 10000, fp);
	    result = strtok(buffer, ",");
	    while (result != NULL) {
	      MI_map_16qam[cnt][table_length]= atof(result);
	      result = strtok(NULL, ",");
	      table_length++;
	    }
	  }
       fclose(fp);
 for (table_len = 0; table_len < 197; table_len++)
	 printf("MIESM 16 QAM Table: %lf  %lf  %1f\n ",MI_map_16qam[0][table_len],MI_map_16qam[1][table_len], MI_map_16qam[2][table_len]);
       break;
	case 64:
	   while (!feof(fp)) {
	    table_length =0;
	    cnt++;
	    if(cnt==3)
	      break;
	    fgets(buffer, 10000, fp);
	    result = strtok(buffer, ",");
	    while (result != NULL) {
	      MI_map_64qam[cnt][table_length]= atof(result);
	      result = strtok(NULL, ",");
	      table_length++;
	    }
	  }
       fclose(fp);
 for (table_len = 0; table_len < 227; table_len++)
	 printf("MIESM 64QAM Table: %lf  %lf  %1f\n ",MI_map_64qam[0][table_len],MI_map_64qam[1][table_len], MI_map_64qam[2][table_len]);
       break;
       
	default:
	  msg("Error, bad input, quitting\n");
	  break;
	}

      }
    }
  free(file_path);
}



