/*!
*******************************************************************************

\file       graph_int.c

\brief      Emulation des interfaces du RRM (Radio Ressource Manager )

            Cette application d'envoyer des stimuli sur les interfaces RRM:
                - RRC -> RRM
                - CMM -> RRM

\author     IACOBELLI Lorenzo

\date       20/04/10


\par     Historique:
            L.IACOBELLI 2009-10-19
                + new messages
            L.IACOBELLI 2010-04-15
                + add sensing unit emulation

*******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <pthread.h>
#include <time.h>

#include "graph_sock.h"
//#include "rrm_sock.h"
#include "graph_int.h"
#include "graph_enum.h"




#define msg_fct printf
#define msg printf
#define NB_SENS_MAX 10

sock_rrm_t S_graph;
static int flag_not_exit = 1 ;
//mod_lor_10_04_21++
typedef struct {
    unsigned int        NB_chan              ; //!< Number of channels 
    unsigned int        channels[NB_SENS_MAX]; //!< Vector of channels
    unsigned int        free[NB_SENS_MAX]    ; //!< Vector of values

} gen_sens_info_t ;
//mod_lor_10_04_21--


int rrm_xface_init(int rrm_inst){

  int sock ; 
  printf("[RRM_XFACE] init de l'interface ");
  sleep(3);
  if(open_socket(&S_graph, RRM_SOCK_PATH, RRM_SOCK_PATH,rrm_inst)==-1)
    return (-1);
  
  if (S_graph.s  == -1) 
    {
      return (-1);
    }
  

  printf("Graphical Interface Connected... RRM of node %d on socket %d\n",rrm_inst, S_graph.s);  
  return 0 ;	
  
}


main(int argc,char **argv) {
    int c = 0;
    int rrm_inst=-1;
    while ((c = getopt(argc,argv,"i:")) != -1)
        switch (c)
        {
            case 'i':
                rrm_inst=atoi(optarg);
            break;
           
            default:
                exit(0);
        }
    //mod_lor_10_04_21++
    //int colorbg; 
    int colorfg; 
    int colorBTS_msg = 30;
    int colorfree=2;
    int colorbusy=9;
    int comments=88;//mod_lor_10_04_22
    int i;
    if (rrm_inst == 0){
        //colorbg = 223;
        colorfg = 21;
        //dbg_color++
        /*for (i=0; i<256; i++){
            printf("\e[38;5;%dm",i);
            printf("%d ",i);
        
        }//dbg_color--*/
    }
    else if (rrm_inst == 1){
        //colorbg = 103;
        colorfg = 11;
    }
    else{
        //colorbg = 223;
        colorfg = 21;
    }
    printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_21
    //char esc=27;
    //printf("%c[1m",esc);   //mod_lor_10_04_22
    //mod_lor_10_04_21--
    printf("MAIN Graphical Interface Connected\n");

    rrm_xface_init(rrm_inst);
    msg_head_t *Header ;
    char *Data;
    unsigned short Data_to_read;

    while (1){
        Header = (msg_head_t *) recv_msg(&S_graph) ;
        if (Header == NULL){
            break;
            
        }
        //mod_lor_10_04_21++
        Data_to_read=Header->size;
        if (Data_to_read > 0 ){
            Data = (char *) (Header +1) ;
        }
        //mod_lor_10_04_21--
        INTERF_T msg_interf;
        int msg_type = Header->msg_type;

        if (Header->msg_type<NB_MSG_SNS_RRM)    
            msg_interf=SNS;
        else if ((msg_type-=NB_MSG_SNS_RRM)< NB_MSG_RRC_RRM)
            msg_interf=RRC;
        else if ((msg_type-=NB_MSG_RRC_RRM)< NB_MSG_CMM_RRM)
            msg_interf=CMM;
        else {
            printf("Error! Unknown message %d!!!\n",Header->msg_type);
            break;
        }

       // msg("Got MSG of Type %d on Inst %d\n",Header->msg_type,Header->inst);
        switch ( msg_interf )
        { 
            case SNS:{
                switch ( msg_type )
                {
                    case SNS_UPDATE_SENS :
                        {
                            msg_fct( "[SENSING]>[RRM]:%d:SNS_UPDATE_SENS transaction number:%d\n",Header->inst, Header->Trans_id) ;
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "Sensing information received from sensing unit\n");
                            msg_fct( "Updating of local sensing database with recived data:\n");

                            //mod_lor_10_04_21++
                            gen_sens_info_t  *p = (gen_sens_info_t  *)Data ;
                            for (i=0;i<p->NB_chan;i++){
                                if(p->free[i]==1){
                                    printf("\e[38;5;%dm",colorfree);   //mod_lor_10_04_21
                                    msg_fct( "      Channel %d: no primary user detected\n",p->channels[i]);
                                    //printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_21
                                }
                                else{
                                    printf("\e[38;5;%dm",colorbusy);   //mod_lor_10_04_21
                                    msg_fct( "      Channel %d: primary user detected\n",p->channels[i]);
                                    //printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_21
                                }
                            }
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "Trasmission of the new information to the Fusion Center\n");
                            msg_fct( "Waiting for next sensing update ...\n");
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_21
                            //mod_lor_10_04_21--
                            
                        }
                        break ;
                    case SNS_END_SCAN_CONF :
                        {
                            
                            msg_fct( "[SENSING]>[RRM]:%d:SNS_END_SCAN_CONF\n",Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "Sending confirmation of stopped sensing to Fusion Center ...\n");
                        }
                        break ;
           
                    default :
                    msg("[SNS]WARNING: msg unknown %d switched as %d\n",Header->msg_type,msg_type) ;
                }
                break;
            }
            case RRC:{
                switch ( msg_type )
                {
                    case RRC_RB_ESTABLISH_RESP:
                        {
                            msg_fct( "[RRC]>[RRM]:%d:RRC_RB_ESTABLISH_RESP \n",Header->inst);
                            
                        }
                        break ;
                    case RRC_RB_ESTABLISH_CFM:
                        {
                            
                            msg_fct( "[RRC]>[RRM]:%d:RRC_RB_ESTABLISH_CFM \n",Header->inst);
                            
                        }
                        break ;

                    case RRC_RB_MODIFY_RESP:
                        {
                            msg_fct( "[RRC]>[RRM]:%d:RRC_RB_MODIFY_RESP \n",Header->inst);
                            
                        }
                        break ;
                    case RRC_RB_MODIFY_CFM:
                        {

                            msg_fct( "[RRC]>[RRM]:%d:RRC_RB_MODIFY_CFM\n",Header->inst);
                            
                        }
                        break ;

                    case RRC_RB_RELEASE_RESP:
                        {
                            msg_fct( "[RRC]>[RRM]:%d:RRC_RB_RELEASE_RESP \n",Header->inst);
                            
                        }
                        break ;
                    case RRC_MR_ATTACH_IND :
                        {
                            
                            msg_fct( "[RRC]>[RRM]:%d:RRC_MR_ATTACH_IND \n",Header->inst);
                            
                        }
                        break ;
                    case RRC_SENSING_MEAS_RESP:
                        {
                            msg_fct( "[RRC]>[RRM]:%d:RRC_SENSING_MEAS_RESP \n",Header->inst);
                        }
                        break ;
                    case RRC_CX_ESTABLISH_IND:
                        {
                    
                            msg_fct( "[RRC]>[RRM]:%d:RRC_CX_ESTABLISH_IND \n",Header->inst);
                            
                        }
                        break ;
                    case RRC_PHY_SYNCH_TO_MR_IND :
                        {
                           
                            msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_MR_IND.... \n",Header->inst);
                            
                        }
                        break ;
                    case RRC_PHY_SYNCH_TO_CH_IND :
                        {
                            msg_fct( "[RRC]>[RRM]:%d:RRC_PHY_SYNCH_TO_CH_IND.... \n",Header->inst);
                            

                        }
                        break ;
                    case RRC_SENSING_MEAS_IND :
                        {

                            msg_fct( "[RRC]>[RRM]:%d:RRC_SENSING_MEAS_IND \n",Header->inst);
                            
                        }
                        break ;
                    case RRC_RB_MEAS_IND :
                        {
                
                            msg_fct( "[RRC]>[RRM]:%d:RRC_RB_MEAS_IND \n",Header->inst);
                        }
                        break ;


                    case RRC_INIT_SCAN_REQ :
                        {
                            msg_fct( "[RRC]>[RRM]:RRC_INIT_SCAN_REQ \n",Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "Order to start sensing activity received from Fusion Center\n");
                            msg_fct( "Activation of sensing unit ...\n");
                            msg_fct( "Waiting for sensing results ...\n");
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_22
                            
                            
                        }
                        break ;
                    case RRC_END_SCAN_CONF :
                        {
                           
                            msg_fct( "[RRC]>[RRM]:RRC_END_SCAN_CONF \n",Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "Sensor %d confirms the end of sensing activity\n",(Header->inst - 1));
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_22
                        }
                        break ;
                    case RRC_END_SCAN_REQ :
                        {
                           
                            msg_fct( "[RRC]>[RRM]:RRC_END_SCAN_REQ \n",Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "Order to stop sensing activity received from Fusion Center\n");
                            msg_fct( "Command to stop sensing activity transmitted to sensing unit\n");
                            msg_fct( "Waiting for confirmation from sensing unit ...\n");
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_22
                            
                        }
                        break ;
                    case RRC_INIT_MON_REQ :
                        {
                            msg_fct( "[RRC]>[RRM]:RRC_INIT_MON_REQ \n ",Header->inst);
                        }
                        break ;
                    case UPDATE_SENS_RESULTS_3 :
                        {
                            
                            msg_fct( "[SENSOR %d msg]:UPDATE_SENS_RESULTS_3 transaction number:%d\n",(Header->inst-1), Header->Trans_id);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "Sensing information received from sensor %d corresponding to sensor local update %d\n",(Header->inst-1),(Header->Trans_id - 4096));
                            msg_fct( "Updating of sensing database using recived data:\n");
                            //mod_lor_10_04_21++
                            gen_sens_info_t  *p = (gen_sens_info_t  *)Data ;
                            for (i=0;i<p->NB_chan;i++){
                                if(p->free[i]==1){
                                    printf("\e[38;5;%dm",colorfree);   //mod_lor_10_04_21
                                    msg_fct( "      Channel %d: no primary user detected\n",p->channels[i]);
                                    printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_21
                                }
                                else{
                                    printf("\e[38;5;%dm",colorbusy);   //mod_lor_10_04_21
                                    msg_fct( "      Channel %d: primary user detected\n",p->channels[i]);
                                    printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_21
                                }
                            }
                            //mod_lor_10_04_21--
                            
                            
                        }
                        break ;
                    case OPEN_FREQ_QUERY_4 :
                        {
                            printf("\e[38;5;%dm",colorBTS_msg);  //mod_lor_10_04_23
                            msg_fct( "[BTS msg]:OPEN_FREQ_QUERY_4 from %d\n",Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_23
                            msg_fct( "Received a request from the secondary network BTS to know the available frequencies to use\n");
                            msg_fct( "Sending to BTS information about available frequencies\n");
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_23
                            
                        }
                        break ;
                    case UPDATE_OPEN_FREQ_7 :
                        {

                            msg_fct( "[FC msg]:UPDATE_OPEN_FREQ_7 from %d\n",Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_23
                            msg_fct( "Received information about available frequencies from Fusion Center\n");
                            msg_fct( "Updating of local channel database ...\n");
                            msg_fct( "Decision about frequencies to use ...\n");
                            msg_fct( "Sending update of frequencies used by Secondary Network to Fusion Center ...\n");
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_23
                            
                            
                        }
                        break ;
                    case UPDATE_SN_OCC_FREQ_5 :
                        {
                            printf("\e[38;5;%dm",colorBTS_msg);  //mod_lor_10_04_23
                            msg_fct( "[BTS msg]:UPDATE_SN_OCC_FREQ_5 from %d\n", Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "Update received from BTS about frequencies used by Secondary Network\n");
                            msg_fct( "Updating CHANNEL DATABASE ...\n");
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_23
                            
                        }
                        break ;
           
                    default :
                    msg("[RRC]WARNING: msg unknown %d switched as %d\n",Header->msg_type,msg_type) ;
                }
                break;
            }
            case CMM:{
                switch ( msg_type )
                {
                    case CMM_CX_SETUP_REQ:
                        {
                            
                            msg_fct( "[CMM]>[RRM]:%d:CMM_CX_SETUP_REQ\n",Header->inst);
                            
                            }
                        break ;
                    case CMM_CX_MODIFY_REQ:
                        {
                            
                            msg_fct( "[CMM]>[RRM]:%d:CMM_CX_MODIFY_REQ\n",Header->inst);
                            
                        }
                        break ;
                    case CMM_CX_RELEASE_REQ :
                        {
                           
                            msg_fct( "[CMM]>[RRM]:%d:CMM_CX_RELEASE_REQ\n",Header->inst);
                            
                        }
                        break ;
                    case CMM_CX_RELEASE_ALL_REQ :
                        {
    
                            msg_fct( "[CMM]>[RRM]:%d:CMM_CX_RELEASE_ALL_REQ\n",Header->inst);
                            
                        }
                        break ;
                    case CMM_ATTACH_CNF : 
                        {
                            msg_fct( "[CMM]>[RRM]:%d:CMM_ATTACH_CNF\n",Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "******************************************************************\n");
                            msg_fct( "The sensor is now connected to the fusion center\n");
                            msg_fct( "******************************************************************\n");
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_22
                        }
                        break ;
                    case CMM_INIT_MR_REQ :
                        {
                            msg_fct( "[CMM]>[RRM]:%d:CMM_INIT_MR_REQ \n",Header->inst);
          
                        }
                        break ;
                    case CMM_INIT_CH_REQ :
                        {
                            msg_fct( "[CMM]>[RRM]:%d:CMM_INIT_CH_REQ \n",Header->inst);
                            
                            
                        }
                        break ;

                    case CMM_INIT_SENSING :
                        {
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "******************************************************************\n"); 
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_22           
                            msg_fct( "[CMM]>[RRM]:%d:CMM_INIT_SENSING\n",Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22
                            msg_fct( "Order to start sensing received \n");
                            msg_fct( "Sending sensing parameters to sensors connected ...\n");
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_22
                        }
                        break ;
                    case CMM_STOP_SENSING :
                        {
                            msg_fct( "[CMM]>[RRM]:%d:CMM_STOP_SENSING\n",Header->inst);
                            printf("\e[38;5;%dm",comments);   //mod_lor_10_04_22 
                            msg_fct( "Order to stop sensing received \n");
                            msg_fct( "Sending order to stop sensing actions to sensors connected ...\n");
                            sleep(1);
                            msg_fct( "Waiting for stop sensing confirmations ...\n");
                            printf("\e[38;5;%dm",colorfg);   //mod_lor_10_04_22
                          
                        }
                        break ;
                    case CMM_ASK_FREQ :
                        {
                            msg_fct( "[CMM]>[RRM]:%d:CMM_ASK_FREQ\n",Header->inst);
                           
                        }
                        break ;

           
                    default :
                    msg("[CMM]WARNING: msg unknown %d switched as %d\n",Header->msg_type,msg_type) ;
                }
                break;
            }
   
            default :
            msg("[Graph_xface]WARNING: msg unknown %d switched as %d\n",Header->msg_type,msg_type) ;

        }
    }
    close_socket(&S_graph) ;
}
