#include "PHY/defs.h"
#include "PHY/impl_defs_lte.h"

double ratioPB[2][4]={{ 1.0,4.0/5.0,3.0/5.0,2.0/5.0},
					  { 5.0/4.0,1.0,3.0/4.0,1.0/2.0}};
					  
double pa_values[8]={-6.0,-4.77,-3.0,-1.77,0.0,1.0,2.0,3.0};

double get_pa_dB(PDSCH_CONFIG_DEDICATED *pdsch_config_dedicated) {
    return(pa_values[pdsch_config_dedicated->p_a]);
} 

double computeRhoA_eNB(PDSCH_CONFIG_DEDICATED *pdsch_config_dedicated,  
                       LTE_eNB_DLSCH_t *dlsch_eNB ){		    	
	double rho_a_dB;
	double sqrt_rho_a_lin;

	rho_a_dB = pa_values[ pdsch_config_dedicated->p_a];
	
    if(!dlsch_eNB->dl_power_off) 
		rho_a_dB-=10*log10(2);
	
	sqrt_rho_a_lin= pow(10,(0.05*rho_a_dB));	
	
	dlsch_eNB->sqrt_rho_a= (short) (sqrt_rho_a_lin*pow(2,13));
	
	
	//	printf("sqrt_rho_a:%d(eNB)\n",dlsch_eNB->sqrt_rho_a);	
	
	return(rho_a_dB);		
}

double computeRhoB_eNB(PDSCH_CONFIG_DEDICATED  *pdsch_config_dedicated,
                       PDSCH_CONFIG_COMMON *pdsch_config_common,
                       u8 n_antenna_port,
                       LTE_eNB_DLSCH_t *dlsch_eNB){

	double rho_a_dB, rho_b_dB;
	double sqrt_rho_b_lin;
	
	rho_a_dB= computeRhoA_eNB(pdsch_config_dedicated,dlsch_eNB);
	
	if(n_antenna_port>1)
		rho_b_dB= ratioPB[1][pdsch_config_common->p_b] * rho_a_dB;
	else
		rho_b_dB= ratioPB[0][pdsch_config_common->p_b] * rho_a_dB;

	sqrt_rho_b_lin= pow(10,(0.05*rho_b_dB));	
	
	dlsch_eNB->sqrt_rho_b= (short) (sqrt_rho_b_lin*pow(2,13));

			
	//	printf("sqrt_rho_b(eNB):%d\n",dlsch_eNB->sqrt_rho_b);
	
	return(rho_b_dB);	
}


double computeRhoA_UE(PDSCH_CONFIG_DEDICATED *pdsch_config_dedicated,  
                      LTE_UE_DLSCH_t *dlsch_ue){		    	
    
	double rho_a_dB;
	double sqrt_rho_a_lin;

	rho_a_dB = pa_values[ pdsch_config_dedicated->p_a];
	
    if(!dlsch_ue->dl_power_off) 
		rho_a_dB-=10*log10(2);
	
	sqrt_rho_a_lin= pow(10,(0.05*rho_a_dB));	
	
	dlsch_ue->sqrt_rho_a= (short) (sqrt_rho_a_lin*pow(2,13));
	
	
	//	printf("sqrt_rho_a(ue):%d\n",dlsch_ue->sqrt_rho_a);	
	
	return(rho_a_dB);		
}

double computeRhoB_UE(PDSCH_CONFIG_DEDICATED  *pdsch_config_dedicated,
                      PDSCH_CONFIG_COMMON *pdsch_config_common,
                      u8 n_antenna_port,
                      LTE_UE_DLSCH_t *dlsch_ue){

	double rho_a_dB, rho_b_dB;
	double sqrt_rho_b_lin;
	
	rho_a_dB= computeRhoA_UE(pdsch_config_dedicated,dlsch_ue);
	
	if(n_antenna_port>1)
		rho_b_dB= ratioPB[1][pdsch_config_common->p_b] * rho_a_dB;
	else
		rho_b_dB= ratioPB[0][pdsch_config_common->p_b] * rho_a_dB;

	sqrt_rho_b_lin= pow(10,(0.05*rho_b_dB));	
	
	dlsch_ue->sqrt_rho_b= (short) (sqrt_rho_b_lin*pow(2,13));

			
	//	printf("sqrt_rho_b(ue):%d\n",dlsch_ue->sqrt_rho_b);
	
	return(rho_b_dB);	
}



void compute_sqrt_RhoAoRhoB(PDSCH_CONFIG_DEDICATED  *pdsch_config_dedicated,
                            PDSCH_CONFIG_COMMON *pdsch_config_common,
                            u8 n_antenna_port,
                            LTE_UE_DLSCH_t *dlsch_ue){
		
	double rho_a_dB,rho_b_dB;			   		
	double sqrt_rho_a_lin,sqrt_rho_b_lin;
		
	rho_a_dB=computeRhoA_UE(pdsch_config_dedicated,dlsch_ue);
	rho_b_dB=computeRhoB_UE(pdsch_config_dedicated,pdsch_config_common,n_antenna_port,dlsch_ue);
		
	sqrt_rho_a_lin= pow(10,(0.05*rho_a_dB));	
	sqrt_rho_b_lin= pow(10,(0.05*rho_b_dB));	
		
	dlsch_ue->sqrt_rho_aob= (short) ((sqrt_rho_a_lin/sqrt_rho_b_lin)*pow(2,12));
}



