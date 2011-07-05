#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/extern.h"

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

#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#ifdef XFORMS
#include "forms.h"
#include "phy_procedures_sim_form.h"
#endif

#include "oaisim.h"

#define RF


#define PI 3.1416
#define Am 20
#define MCS_COUNT 23
#define MCL 70 /*minimum coupling loss (MCL) in dB*/
#define theta_3dB (65*PI/180)
enum sector {SEC1, SEC2, SEC3};
double sinr[NUMBER_OF_eNB_MAX][2*25];
extern double sinr_bler_map[MCS_COUNT][2][9];


// Extract the positions of UE and ENB from the mobility model 

void extract_position (Node_list input_node_list, node_desc_t **node_data) {    
    
  int index = 0;
  while (input_node_list != NULL) {
    node_data[index]->x = input_node_list->node->X_pos;
    node_data[index++]->y = input_node_list->node->Y_pos;
    input_node_list = input_node_list->next;
  }
}

void init_ue(node_desc_t  *ue_data) {//changed from node_struct
  ue_data->phi_rad = 2 * PI;
  ue_data->tx_power_dBm = 20;
  ue_data->ant_gain_dBi = 0;
  ue_data->rx_noise_level = 9; //value in db
  //ue_data->x=0;// just for the test 
  //ue_data->y=0;
}

void init_enb(node_desc_t  *enb_data) {//changed from node_struct
  enb_data->tx_power_dBm = 40;
  enb_data->ant_gain_dBi = 15;
  enb_data->rx_noise_level = 5; //value in db
  enb_data->n_sectors = 3;
  //enb_data->x=0;// just for the test 
  //enb_data->y=0;	
}

void calc_path_loss(node_desc_t* node_tx, node_desc_t* node_rx, channel_desc_t *ch_desc) {  
  double dist; 
  //dist = sqrt(pow((node_tx->X_pos - node_rx->X_pos), 2) + pow((node_tx->Y_pos - node_rx->Y_pos), 2));
  dist = sqrt(pow((node_tx->x - node_rx->x), 2) + pow((node_tx->y - node_rx->y), 2));
  
  /* conversion of distance into KM 3gpp (36-942) */
  ch_desc->path_loss_dB = (128.1 + 37.6 * log10(dist/10)); 
  //ch_desc->path_loss_dB = (128.1 + 10*(scenario->path_loss_exponent) * log10(dist/1000)); 
  printf("*****Path Loss %f\n",ch_desc->path_loss_dB);
}





void init_snr(channel_desc_t* eNB2UE, node_desc_t *enb_data, node_desc_t *ue_data, s32 UE_id,s32 eNB_id) {

  double sect_angle[3]={0,2*PI/3,4*PI/3};
  double gain_max;
  double theta;
  int count;
  int return_value;
  u16 nb_rb = 25; //No. of resource blocks
  //double sinr[NUMBER_OF_eNB_MAX][2*nb_rb];
  
  double gain_sec[3];
  double thermal_noise;
  double coupling;

  ////////////////////////////////////////////////////////////
  /* Calculating the angle in the range -pi to pi from the slope */
  //(ue_data[UE_id])->alpha_rad[eNB_id] = (double)(atan2((ue_data[UE_id]->x - enb_data[eNB_id]->x), (ue_data[UE_id]->y - enb_data[eNB_id]->y)));
  ue_data->alpha_rad[eNB_id] = atan2((ue_data->x - enb_data->x), (ue_data->y - enb_data->y));
  //printf("angle is tan %lf\n", ue_data[UE_id]->alpha_rad[eNB_id]);
      
  if ((ue_data->alpha_rad[eNB_id]) < 0) {
    ue_data->alpha_rad[eNB_id] = 2*PI + ue_data->alpha_rad[eNB_id]; 
    //printf("angle in radians is %lf\n", ue_data[UE_id]->alpha_rad[eNB_id]);
  }
      
  for(count = 0; count < enb_data->n_sectors; count++) {
    theta = sect_angle[count] - ue_data->alpha_rad[eNB_id];
    gain_sec[count] = -(Am < (12 * pow((theta/theta_3dB),2)) ? Am : (12 * pow((theta/theta_3dB),2)));
  }
      
  /* gain = -min(Am , 12 * (theta/theta_3dB)^2) */
  gain_max = (gain_sec[SEC1] > gain_sec[SEC2]) ? ((gain_sec[SEC1] > gain_sec[SEC3]) ? gain_sec[SEC1]:gain_sec[SEC3]) : 
                                                  ((gain_sec[SEC2] > gain_sec[SEC3]) ? gain_sec[SEC2]:gain_sec[SEC3]); 

  
      
  //printf("Path loss for link between ue %d and enb %d is %lf and gain is %lf \n", UE_id, eNB_id, eNB2UE[UE_id][eNB_id]->path_loss_dB, gain_max); 
  return_value = random_channel(eNB2UE);
      
  /* Thermal noise is calculated using 10log10(K*T*B) K = BoltzmannŽs constant T = room temperature B = bandwidth */
  /* Taken as constant for the time being since the BW is not changing */
  thermal_noise = -105; //value in dBm 
      
  if (0 == return_value) {
    //freq_channel(ul_channel[UE_id][eNB_id], nb_rb);
    freq_channel(eNB2UE, nb_rb);
    coupling = MCL > (eNB2UE->path_loss_dB-(enb_data->ant_gain_dBi + gain_max)) ?
               MCL : (eNB2UE->path_loss_dB-(enb_data->ant_gain_dBi + gain_max));   
    //printf ("coupling factor is %lf\n", coupling); 
    for (count = 0; count < (2 * nb_rb); count++) {
      sinr[eNB_id][count] = enb_data->tx_power_dBm 
		            - coupling  
                            - (thermal_noise + ue_data->rx_noise_level)  
                            + 10 * log10 (pow(eNB2UE->chF[0][count].x, 2) 
                            + pow(eNB2UE->chF[0][count].y, 2));
      sinr[eNB_id][count] *= 0.1; //tweak in order to work with one antenna (to force the value to be in range)
      //printf("Dl_link SNR for res. block %d is %lf\n", count, sinr[eNB_id][count]);
    }
  } 
}//function ends





void calculate_sinr(channel_desc_t* eNB2UE,double *sinr_dB, s32 UE_id, s32 eNB_id, s32 att_eNB_id) {

  double interference;
  short nb_rb = 25; //No. of resource blocks
  short count;
  //double interference;

  for (count = 0; count < 2 * nb_rb; count++) {
    interference = 0;
    sinr_dB[count]=sinr[att_eNB_id][count];
    for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
      if (att_eNB_id != eNB_id) {
        interference += pow(10, 0.1 * sinr[eNB_id][count]);
      }
    }
    sinr_dB[count] -= 10*log10(1 + interference);
    printf("*****sinr% lf \n",sinr_dB[count]);
  }
}
void get_beta_map() {
  char *file_name[] = {"bler_1.csv", "bler_2.csv", "bler_3.csv", "bler_4.csv", "bler_5.csv", "bler_6.csv", "bler_7.csv", "bler_8.csv",
                    "bler_9.csv", "bler_10.csv", "bler_11.csv", "bler_12.csv", "bler_13.csv", "bler_14.csv", "bler_15.csv", "bler_16.csv",
                    "bler_17.csv", "bler_18.csv", "bler_19.csv", "bler_20.csv", "bler_21.csv", "bler_22.csv"};
  
  int table_len = 0;
  int mcs = 0;
  char *sinr_bler;
  char buffer[100];
  FILE *fp;
 
  for (mcs = 0; mcs <= 4; mcs++) {
    for (table_len = 0; table_len < 9; table_len++) {
      sinr_bler_map[mcs][0][table_len] = 0.0;
      sinr_bler_map[mcs][1][table_len] = 0.0;
    }
  }

  for (mcs = 5; mcs < MCS_COUNT; mcs++) {
    fp = fopen(file_name[mcs - 1],"r");
    if (fp == NULL) {
      printf("ERROR: Unable to open the file %s\n", file_name[mcs-1]);
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
}


