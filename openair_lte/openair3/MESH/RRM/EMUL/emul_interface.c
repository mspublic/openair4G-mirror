/*!
*******************************************************************************

\file       emul_interface.c

\brief      Emulation des interfaces du RRM (Radio Ressource Manager )

            Cette application d'envoyer des stimuli sur les interfaces RRM:
                - RRC -> RRM
                - CMM -> RRM

\author     BURLOT Pascal

\date       10/07/08


\par     Historique:
            L.IACOBELLI 2009-10-19
                + new messages

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

#include "debug.h"

#include "L3_rrc_defs.h"
#include "cmm_rrm_interface.h"

#include "rrm_sock.h"
#include "cmm_msg.h"
#include "rrc_rrm_msg.h"
#include "pusu_msg.h"

#include "transact.h"
#include "actdiff.h"
#include "rrm_util.h"
#include "rrm_constant.h"

#define NUM_SCENARIO  2
#define SENSORS_NB 2 //mod_lor_10_03_03
#define PUSU_EMUL

#ifdef RRC_EMUL

extern msg_t *msg_rrc_rb_meas_ind(Instance_t inst, RB_ID Rb_id, L2_ID L2_id, MEAS_MODE Meas_mode, MAC_RLC_MEAS_T *Mac_rlc_meas_t, Transaction_t Trans_id );
extern msg_t *msg_rrc_sensing_meas_ind( Instance_t inst, L2_ID L2_id, unsigned int NB_meas, SENSING_MEAS_T *Sensing_meas, Transaction_t Trans_id );
extern msg_t *msg_rrc_sensing_meas_resp( Instance_t inst, Transaction_t Trans_id )  ;
extern msg_t *msg_rrc_cx_establish_ind( Instance_t inst, L2_ID L2_id, Transaction_t Trans_id,unsigned char *L3_info, L3_INFO_T L3_info_t,
                                    RB_ID DTCH_B_id, RB_ID DTCH_id );
extern msg_t *msg_rrc_phy_synch_to_MR_ind( Instance_t inst, L2_ID L2_id);
extern msg_t *msg_rrc_phy_synch_to_CH_ind( Instance_t inst, unsigned int Ch_index,L2_ID L2_id );
extern msg_t *msg_rrc_rb_establish_resp( Instance_t inst, Transaction_t Trans_id  );
extern msg_t *msg_rrc_rb_establish_cfm( Instance_t inst, RB_ID Rb_id, RB_TYPE RB_type, Transaction_t Trans_id );
extern msg_t *msg_rrc_rb_modify_resp( Instance_t inst, Transaction_t Trans_id );
extern msg_t *msg_rrc_rb_modify_cfm(Instance_t inst, RB_ID Rb_id, Transaction_t Trans_id  );
extern msg_t *msg_rrc_rb_release_resp( Instance_t inst, Transaction_t Trans_id );
extern msg_t *msg_rrc_MR_attach_ind( Instance_t inst, L2_ID L2_id );
extern msg_t *msg_rrc_update_sens( Instance_t inst,  /*double info_time,*/ L2_ID L2_id, unsigned int NB_info, Sens_ch_t *Sens_meas, Transaction_t Trans_id);


#endif

typedef struct {
    L2_ID               L2_id               ; ///< identification de niveau L2
    L3_INFO_T           L3_info_t           ; ///< type de l'identification de niveau L3
    unsigned char       L3_info[MAX_L3_INFO]; ///< identification de niveau L3
} node_info_t ;

/*node_info_t node_info[10] = {
 { .L2_id={{0x00,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} },
 { .L2_id={{0x01,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} },
 { .L2_id={{0x02,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} },
 { .L2_id={{0x03,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} },
 { .L2_id={{0x04,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} },
 { .L2_id={{0x05,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} },
 { .L2_id={{0x06,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} },
 { .L2_id={{0x07,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} },
 { .L2_id={{0x08,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} },
 { .L2_id={{0x09,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv6_ADDR, .L3_info={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11} }
} ;*/
//mod_lor_10_01_25++
node_info_t node_info[10] = {
 { .L2_id={{0x00,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x0A,0x00,0x01,0x01} },
 { .L2_id={{0x01,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x0A,0x00,0x02,0x02} },
 { .L2_id={{0x02,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x0A,0x00,0x03,0x03} },
 { .L2_id={{0x03,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x0A,0x00,0x04,0x04} },
 { .L2_id={{0x04,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x0A,0x00,0x05,0x05} },
 { .L2_id={{0x05,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x05,0x00,0xAA,0xCC} },
 { .L2_id={{0x06,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x06,0x00,0xAA,0xCC} },
 { .L2_id={{0x07,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x07,0x00,0xAA,0xCC} },
 { .L2_id={{0x08,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x08,0x00,0xAA,0xCC} },
 { .L2_id={{0x09,0x00,0xAA,0xCC,0x33,0x55,0x00,0x11}}, .L3_info_t=IPv4_ADDR, .L3_info={0x09,0x00,0xAA,0xCC} }
} ;
//mod_lor_10_01_25--*/

//void print_pusu_msg( neighbor_entry_RRM_to_CMM_t *pEntry );

static int flag_not_exit = 1 ;
int attached_sensors = 0;//mod_lor_10_01_25

static pthread_t        pthread_rrc_hnd,
                        pthread_cmm_hnd ,
                        pthread_pusu_hnd , // Publish Subscribe : -> routing CH
                        pthread_action_differe_hnd;

pthread_mutex_t         cmm_transact_exclu,
                        rrc_transact_exclu;

unsigned int            cmm_transaction=512,
                        rrc_transaction=256 ;

transact_t              *cmm_transact_list=NULL,
                        *rrc_transact_list=NULL ;

static RB_ID rb_id      =4 ;

pthread_mutex_t actdiff_exclu;
actdiff_t       *list_actdiff   = NULL ;
unsigned int    cnt_actdiff     = 512;


extern void scenario(  int num , sock_rrm_t *s_rrc,  sock_rrm_t *s_cmm) ;

/*****************************************************************************
 * \brief  thread d'emulation de l'interface du Publish/subcribe (routingCH).
 * \return NULL
 */
#ifdef PUSU_EMUL
static void *fn_pusu (
    void * p_data /**< parametre du pthread */
    )
{
    sock_rrm_t *s = (sock_rrm_t *) p_data ;
    msg_head_t  *header ;

#ifdef TRACE
    FILE *fd = fopen( "VCD/rrm2pusu.txt", "w") ;
    PNULL(fd) ;
#endif

    fprintf(stderr,"PUSU interfaces :starting ...\n");

    while (flag_not_exit)
    {
        header = (msg_head_t *) recv_msg(s) ;
        if (header == NULL )
        {
            fprintf(stderr,"Server closed connection\n");
            flag_not_exit = 0;
        }
        else
        {
            char *msg = NULL ;

            if ( header->size > 0 )
            {
                msg = (char *) (header +1) ;
            }
#ifdef TRACE
            if ( header->msg_type < NB_MSG_RRM_PUSU )
            fprintf(fd,"%lf RRM->PUSU %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_pusu_rrm[header->msg_type],header->msg_type,header->Trans_id);
            else
            fprintf(fd,"%lf RRM->PUSU %-30s %d %d\n",get_currentclock(), "inconnu", header->msg_type,header->Trans_id);
            fflush(fd);
#endif
            switch ( header->msg_type )
            {
                case RRM_PUBLISH_IND:
                    {
                        msg_fct( "[RRM]>[PUSU]:%d:RRM_PUBLISH_IND\n",header->inst);
                        send_msg( s, msg_pusu_resp( header->inst, PUSU_PUBLISH_RESP, header->Trans_id )) ;
                    }
                    break ;
                case RRM_UNPUBLISH_IND:
                    {
                        msg_fct( "[RRM]>[PUSU]:%d:RRM_UNPUBLISH_IND\n",header->inst);
                        send_msg( s, msg_pusu_resp( header->inst, PUSU_UNPUBLISH_RESP, header->Trans_id )) ;
                    }
                    break ;
                case RRM_LINK_INFO_IND:
                    {
                        msg_fct( "[RRM]>[PUSU]:%d:RRM_LINK_INFO_IND\n",header->inst);
                        send_msg( s, msg_pusu_resp( header->inst, PUSU_LINK_INFO_RESP, header->Trans_id )) ;
                    }
                    break ;
                case RRM_SENSING_INFO_IND:
                    {
                        msg_fct( "[RRM]>[PUSU]:%d:RRM_SENSING_INFO_IND\n",header->inst);
                        send_msg( s, msg_pusu_resp( header->inst, PUSU_SENSING_INFO_RESP, header->Trans_id )) ;
                    }
                    break ;
                case RRM_CH_LOAD_IND:
                    {
                        msg_fct( "[RRM]>[PUSU]:%d:RRM_CH_LOAD_IND\n",header->inst);
                        send_msg( s, msg_pusu_resp( header->inst, PUSU_CH_LOAD_RESP, header->Trans_id )) ;
                    }
                    break ;
                default:
                    fprintf(stderr, "[RRM]>[PUSU]: msg unknown %d\n", header->msg_type) ;
                    //printHex(msg,n,1);
            }
            RRM_FREE(header);
        }
    }

    fprintf(stderr,"... stopped PUSU interfaces\n");
#ifdef TRACE
    fclose(fd) ;
#endif

    return NULL;
}
#endif /* PUSU_EMUL */

#ifdef RRC_EMUL

/*****************************************************************************
 * \brief  thread d'emulation de l'interface du rrc.
 * \return NULL
 */
static void * fn_rrc (
    void * p_data /**< parametre du pthread */
    )
{
    sock_rrm_t *s = (sock_rrm_t *) p_data ;
    msg_head_t *header ;

#ifdef TRACE
    FILE *fd = fopen( "VCD/rrm2rrc.txt", "w") ;
    PNULL(fd) ;
#endif

    fprintf(stderr,"RRC interfaces :starting ...\n");
    fprintf(stderr,"prova\n"); //dbg

    while (flag_not_exit)
    {
        header = (msg_head_t *) recv_msg(s) ;
        if (header == NULL )
        {
            fprintf(stderr,"Server closed connection\n");
            flag_not_exit = 0;
        }
        else
        {
            char *msg = NULL ;

            if ( header->size > 0 )
            {
                msg = (char *) (header +1) ;
            }
#ifdef TRACE
            if ( header->msg_type < NB_MSG_RRC_RRM )
            fprintf(fd,"%lf RRM->RRC %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_rrc_rrm[header->msg_type],header->msg_type,header->Trans_id);
            else
            fprintf(fd,"%lf RRM->RRC %-30s %d %d\n",get_currentclock(), "inconnu", header->msg_type,header->Trans_id);
            fflush(fd);
#endif
            switch ( header->msg_type )
            {
                case RRM_RB_ESTABLISH_REQ:
                    {
                        //rrm_rb_establish_req_t *p = (rrm_rb_establish_req_t *) msg ;
                        msg_fct( "[RRM]>[RRC]:%d:RRM_RB_ESTABLISH_REQ\n",header->inst);

                        send_msg( s, msg_rrc_rb_establish_resp( header->inst, header->Trans_id )) ;

                        pthread_mutex_lock( &actdiff_exclu  ) ;

                        add_actdiff(&list_actdiff,0.05, cnt_actdiff++,  s,
                                    msg_rrc_rb_establish_cfm( header->inst, rb_id++, UNICAST,header->Trans_id) ) ;

                        pthread_mutex_unlock( &actdiff_exclu ) ;
                    }
                    break ;
                case RRM_RB_MODIFY_REQ:
                    {
                        rrm_rb_modify_req_t *p = (rrm_rb_modify_req_t *) msg ;
                        msg_fct( "[RRM]>[RRC]:%d:RRM_RB_MODIFY_REQ\n",header->inst);

                        send_msg( s, msg_rrc_rb_modify_resp( header->inst,header->Trans_id )) ;

                        pthread_mutex_lock( &actdiff_exclu  ) ;

                        add_actdiff(&list_actdiff,0.05, cnt_actdiff++,  s,
                                    msg_rrc_rb_modify_cfm( header->inst, p->Rb_id, header->Trans_id) ) ;

                        pthread_mutex_unlock( &actdiff_exclu ) ;

                    }
                    break ;

                case RRM_RB_RELEASE_REQ:
                    {
                        //rrm_rb_release_req_t *p = (rrm_rb_release_req_t *) msg ;
                        msg_fct( "[RRM]>[RRC]:%d:RRM_RB_RELEASE_REQ\n",header->inst);

                        send_msg( s, msg_rrc_rb_release_resp( header->inst,header->Trans_id )) ;
                    }
                    break ;

                case RRM_SENSING_MEAS_REQ:
                    {
                        //rrm_sensing_meas_req_t *p = (rrm_sensing_meas_req_t *) msg ;
                        msg_fct( "[RRM]>[RRC]:%d:RRM_SENSING_MEAS_REQ\n",header->inst);

                        send_msg( s, msg_rrc_sensing_meas_resp( header->inst, header->Trans_id ) );
                    }
                    break ;

                case RRCI_CX_ESTABLISH_RESP:
                    {
                        rrci_cx_establish_resp_t *p = (rrci_cx_establish_resp_t *) msg ;
                        msg_fct( "[RRCI]>[RRC]:%d:RRCI_CX_ESTABLISH_RESP\n",header->inst);

                        fprintf(stderr,"L3_id: ");
                        print_L3_id(p->L3_info_t, p->L3_info );
                        fprintf(stderr,"\n");

                    }
                    break ;

                case RRM_SENSING_MEAS_RESP:
                    {
                        msg_fct( "[RRM]>[RRC]:%d:RRM_SENSING_MEAS_RESP\n",header->inst);
                    }
                    break ;

                case RRM_RB_MEAS_RESP:
                    {
                        msg_fct( "[RRM]>[RRC]:%d:RRM_RB_MEAS_RESP\n",header->inst);
                    }
                    break ;

                case RRM_INIT_CH_REQ:
                    {
                        msg_fct( "[RRM]>[RRC]:%d:RRM_INIT_CH_REQ\n",header->inst);
                    }
                    break ;

                case RRCI_INIT_MR_REQ:
                    {
                        msg_fct( "[RRM]>[RRC]:%d:RRM_INIT_MR_REQ\n",header->inst);
                    }
                    break ;
                case RRM_INIT_MON_REQ:
                    {
                        rrm_init_mon_req_t *p = (rrm_init_mon_req_t *) msg ;
                        msg_fct( "[RRM]>[RRC]:%d:RRM_INIT_MON_REQ on channels: ",header->inst);
                        fprintf(stdout,"chan nb: %d\n", p->NB_chan); //dbg
                        for ( int i=0;i<p->NB_chan;i++)
                            msg_fct("%d ", p->ch_to_scan[i]);
                        msg_fct( "\n");
                    
                    }
                    break ;
                case RRM_INIT_SCAN_REQ:
                    {
                        msg_fct( "[RRM]>[RRC]:%d:RRM_INIT_SCAN_REQ\n",header->inst);
                        
                    }
                    break ;
                case RRM_SCAN_ORD:
                    {
                        msg_fct( "[RRM]>[RRC]:%d:RRM_SCAN_ORD\n",header->inst);
                        
                    }
                    break ;
                case RRM_END_SCAN_REQ:
                    {
                        rrm_end_scan_req_t *p = (rrm_end_scan_req_t *) msg ;
                        msg_fct( "[RRM]>[RRC]:%d:RRM_END_SCAN_REQ on sensor",header->inst);
                        for ( int i=0;i<8;i++)
                            msg_fct("%02X", p->L2_id.L2_id[i]);
                        msg_fct( "\n");
                        
                        
                    }
                    break ;
                case RRM_END_SCAN_ORD:
                    {
                        msg_fct( "[RRM]>[RRC]:%d:RRM_END_SCAN_ORD\n",header->inst);
                        
                    }
                    break ;
                case RRM_UP_FREQ_ASS :
                    {
                        msg_fct( "[RRM]>[RRC]:%d:RRM_UP_FREQ_ASS\n",header->inst);   
                    }
                    break ;

                default :
                    fprintf(stderr, "RRC: msg unknown %d\n", header->msg_type) ;
                    //printHex(msg,n,1);
            }

            RRM_FREE(header);
        }

    }

    fprintf(stderr,"... stopped RRC interfaces\n");

#ifdef TRACE
    fclose(fd) ;
#endif

    return NULL;
}

#endif /* RRC_EMUL */

/*****************************************************************************
 * \brief  thread d'emulation de l'interface du cmm.
 * \return NULL
 */
static void * fn_cmm (
    void * p_data /**< parametre du pthread */
    )
{
    sock_rrm_t *s = (sock_rrm_t *) p_data ;
    msg_head_t *header ;

#ifdef TRACE
    FILE *fd = fopen( "VCD/rrm2cmm.txt", "w") ;
    PNULL(fd) ;
#endif

    fprintf(stderr,"CMM interfaces :starting ...\n");

    while (flag_not_exit)
    {
        header = (msg_head_t *) recv_msg(s) ;
        if (header == NULL )
        {
            fprintf(stderr,"Server closed connection\n");
            flag_not_exit = 0;
        }
        else
        {
            char *msg = NULL ;

            if ( header->size > 0 )
            {
                msg = (char *) (header +1) ;
            }
#ifdef TRACE
            if ( header->msg_type < NB_MSG_CMM_RRM )
            fprintf(fd,"%lf RRM->CMM %d %-30s %d %d\n",get_currentclock(),header->inst,Str_msg_cmm_rrm[header->msg_type],header->msg_type,header->Trans_id);
            else
            fprintf(fd,"%lf RRM->CMM %-30s %d %d\n",get_currentclock(),"inconnu",header->msg_type,header->Trans_id);
            fflush(fd);
#endif
            switch ( header->msg_type )
            {
                case RRM_CX_SETUP_CNF :
                    {
                        // rrm_cx_setup_cnf_t *p = (rrm_cx_setup_cnf_t *) msg ;
                        msg_fct( "[RRM]>[CMM]:%d:RRM_CX_SETUP_CNF\n",header->inst);

                        pthread_mutex_lock( &cmm_transact_exclu ) ;
                        del_item_transact( &cmm_transact_list, header->Trans_id );
                        pthread_mutex_unlock( &cmm_transact_exclu ) ;
                    }
                    break ;
                case RRM_CX_MODIFY_CNF :
                    {
                        //rrm_cx_modify_cnf_t *p = (rrm_cx_modify_cnf_t *) msg ;
                        msg_fct( "[RRM]>[CMM]:%d:RRM_CX_MODIFY_CNF\n",header->inst);
                    }
                    break ;
                case RRM_CX_RELEASE_CNF :
                    {
                        //rrm_cx_release_cnf_t *p = (rrm_cx_release_cnf_t *) msg ;
                        msg_fct( "[RRM]>[CMM]:%d:RRM_CX_RELEASE_CNF\n",header->inst);
                    }
                    break ;
                case RRM_CX_RELEASE_ALL_CNF :
                    {
                        //rrm_cx_release_all_cnf_t *p = (rrm_cx_release_all_cnf_t *) msg ;
                        msg_fct( "[RRM]>[CMM]:%d:RRM_CX_RELEASE_ALL_CNF\n",header->inst);
                    }
                    break ;
                case RRCI_ATTACH_REQ :
                    {
#ifndef PHY_EMUL
                        float delai = 0.05 ;
#else
                        float delai = 0.00 ;
#endif
                        rrci_attach_req_t *p = (rrci_attach_req_t *) msg ;
                        msg_fct( "[RRM]>[CMM]:%d:RRCI_ATTACH_REQ \n",header->inst);
                        //MSG_L2ID(p->L2_id);
                        pthread_mutex_lock( &actdiff_exclu  ) ;

                        add_actdiff(&list_actdiff,delai, cnt_actdiff++, s,
                                msg_cmm_attach_cnf(header->inst,p->L2_id,node_info[header->inst].L3_info_t,node_info[header->inst].L3_info,header->Trans_id ) ) ;

                        pthread_mutex_unlock( &actdiff_exclu ) ;
                    }
                    break ;
                case RRM_ATTACH_IND :
                    { //mod_lor: 10_02_09++
                        if (WSN && header->inst == 0) //inst_to_change: remove header->inst == 0 in case WSN and SN not on the same machine
                            attached_sensors ++;//mod_lor: 10_01_25
                        //msg_fct( "attached_sensors %d \n\n",attached_sensors); //dbg
#ifndef PHY_EMUL
                        float delai = 0.05 ;
#else
                        float delai = 0.00 ;
#endif
                        msg_fct( "[RRM]>[CMM]:%d:RRM_ATTACH_IND\n",header->inst);
                        
                        if (WSN && attached_sensors==SENSORS_NB && header->inst == 0){ //inst_to_change: remove header->inst == 0 in case WSN and SN not on the same machine

                            pthread_mutex_lock( &actdiff_exclu  ) ; 
                            add_actdiff(&list_actdiff,5, cnt_actdiff++, s,
                                    msg_cmm_init_sensing(header->inst,1 ) );

                            pthread_mutex_unlock( &actdiff_exclu ) ;  //mod_lor: 10_02_09--
                            //msg_fct( "\npassato CH %d \n\n",header->inst); //dbg
                            pthread_mutex_lock( &actdiff_exclu  ) ; 
                            add_actdiff(&list_actdiff,20, cnt_actdiff++, s,
                                    msg_cmm_stop_sensing(0) );

                            pthread_mutex_unlock( &actdiff_exclu ) ;  //mod_lor: 10_02_09--*/
                        } 

                    }
                    break ;
                case RRM_MR_ATTACH_IND :
                    {
                        L2_ID L2_id_mr;
#ifndef PHY_EMUL
                        float delai = 0.05 ;
#else
                        float delai = 0.00 ;
#endif
                        rrm_MR_attach_ind_t *p = (rrm_MR_attach_ind_t *) msg ;

                        msg_fct( "[RRM]>[CMM]:%d:RRM_MR_ATTACH_IND\n",header->inst);
                        memcpy( L2_id_mr.L2_id, p->L2_id.L2_id, sizeof(L2_ID));

                        pthread_mutex_lock( &actdiff_exclu  ) ;

                        cmm_transaction++;
                        add_actdiff(&list_actdiff,delai, cnt_actdiff++, s,
                                    msg_cmm_cx_setup_req(header->inst,node_info[header->inst].L2_id,L2_id_mr, QOS_DTCH_D, cmm_transaction ) ) ;

                        pthread_mutex_unlock( &actdiff_exclu ) ;

                        pthread_mutex_lock( &cmm_transact_exclu ) ;
                        add_item_transact( &cmm_transact_list, cmm_transaction, INT_CMM,CMM_CX_SETUP_REQ,0,NO_PARENT);
                        pthread_mutex_unlock( &cmm_transact_exclu ) ;
                    }
                    break ;
                case ROUTER_IS_CH_IND :
                    {
#ifndef PHY_EMUL
                        float delai  = 0.05 ;
                        float delai2 = 0.08 ;
#else
                        float delai  = 0.00 ;
                        float delai2 = 0.00 ;
#endif
                        router_is_CH_ind_t *p =(router_is_CH_ind_t *)msg ;
                        msg_fct( "[RRM]>[CMM]:%d:ROUTER_IS_CH_IND\n",header->inst);

                        memcpy( node_info[header->inst].L2_id.L2_id, p->L2_id.L2_id, sizeof(L2_ID));
                        //print_L2_id(&L2_id_ch ); printf("=>L2_id_ch\n");

                        pthread_mutex_lock( &actdiff_exclu  ) ;

                        add_actdiff(&list_actdiff,delai, cnt_actdiff++,  s,
                                    msg_cmm_init_ch_req( header->inst,node_info[header->inst].L3_info_t,node_info[header->inst].L3_info )) ;

                        cmm_transaction++;
                        add_actdiff(&list_actdiff,delai2, cnt_actdiff++, s,
                                    msg_cmm_cx_setup_req(header->inst,node_info[header->inst].L2_id,node_info[header->inst].L2_id, QOS_DTCH_B, cmm_transaction ) ) ;

                        pthread_mutex_unlock( &actdiff_exclu ) ;

                        pthread_mutex_lock( &cmm_transact_exclu ) ;
                        add_item_transact( &cmm_transact_list, cmm_transaction, INT_CMM, CMM_CX_SETUP_REQ,0,NO_PARENT);
                        pthread_mutex_unlock( &cmm_transact_exclu ) ;
                        
                        if (header->inst==1){
                            pthread_mutex_lock( &actdiff_exclu  ) ; 
                            add_actdiff(&list_actdiff,15, cnt_actdiff++, s, msg_cmm_ask_freq(header->inst) );
                            pthread_mutex_unlock( &actdiff_exclu ) ;
                        }
                    }
                    break ;
                case RRCI_CH_SYNCH_IND :
                    {
                        msg_fct( "[RRM]>[CMM]:%d:RRCI_CH_SYNCH_IND\n",header->inst);
                    }
                    break ;
                case RRM_MR_SYNCH_IND :
                    {
                        msg_fct( "[RRM]>[CMM]:%d:RRM_MR_SYNCH_IND\n",header->inst);
                    }
                    break ;
                case RRM_NO_SYNCH_IND:
                    {
                        msg_fct( "[RRM]>[CMM]:%d:RRM_NO_SYNCH_IND\n",header->inst);
                    }
                    break ;
                default :
                    fprintf(stderr, "CMM:unknown msg %d\n", header->msg_type) ;
                    //printHex(msg,n,1);
            }
            RRM_FREE(header);
        }
    }

    fprintf(stderr,"... stopped CMM interfaces\n");

#ifdef TRACE
    fclose(fd) ;
#endif

    return NULL;
}

/*****************************************************************************
 * \brief  thread d'emulation de l'interface du cmm.
 * \return NULL
 */
static void * fn_action_differe (
    void * p_data /**< parametre du pthread */
    )
{
    fprintf(stderr,"thread action differe :starting ...\n");

    while (flag_not_exit)
    {
        usleep( 10*1000 ) ;

        //  traitement de liste d'actions differees
        pthread_mutex_lock( &actdiff_exclu  ) ;
        processing_actdiff(&list_actdiff ) ;
        pthread_mutex_unlock( &actdiff_exclu ) ;

    }
    fprintf(stderr,"... stopped thread action differe\n");
    return NULL;
}

int main( int argc , char **argv )
{
    int ret = 0;
#ifdef RRC_EMUL
    sock_rrm_t s_rrc ;
#endif /* RRC_EMUL */
    sock_rrm_t s_cmm ;
    sock_rrm_t s_pusu ;

    /* ***** MUTEX ***** */
    pthread_attr_t attr ;

    // initialise les attributs des threads
    pthread_attr_init( &attr ) ;
    pthread_attr_setschedpolicy( &attr, SCHED_RR ) ;

    pthread_mutex_init( &actdiff_exclu      , NULL ) ;
    pthread_mutex_init( &cmm_transact_exclu , NULL ) ;
    pthread_mutex_init( &rrc_transact_exclu , NULL ) ;

    fprintf(stderr,"Emulation des interfaces\n");

#ifdef RRC_EMUL
    fprintf(stderr,"Trying to connect... RRM-RRC\n");
    open_socket(&s_rrc, RRC_RRM_SOCK_PATH, RRM_RRC_SOCK_PATH,0) ;
    if (s_rrc.s  == -1)
        exit(1);
    fprintf(stderr,"Connected... RRM-RRC (s=%d)\n",s_rrc.s);
#endif /* RRC_EMUL */

    fprintf(stderr,"Trying to connect... RRM-CMM\n");
    open_socket(&s_cmm,CMM_RRM_SOCK_PATH,RRM_CMM_SOCK_PATH,0) ;
    if (s_cmm.s  == -1)
        exit(1);
    fprintf(stderr,"Connected... RRM-CMM (s=%d)\n",s_cmm.s);

#ifdef PUSU_EMUL
    fprintf(stderr,"Trying to connect... RRM-PUSU\n");
    open_socket(&s_pusu,PUSU_RRM_SOCK_PATH,RRM_PUSU_SOCK_PATH,0) ;
    if (s_pusu.s  == -1)
        exit(1);
    fprintf(stderr,"Connected... RRM-PUSU (s=%d)\n",s_pusu.s);
#endif

#ifdef RRC_EMUL
   /* Creation du thread RRC */
    fprintf(stderr,"Creation du thread RRC \n");
    ret = pthread_create ( &pthread_rrc_hnd, NULL, fn_rrc, &s_rrc );
    if (ret)
    {
        fprintf(stderr, "%s", strerror (ret));
    }
#endif /* RRC_EMUL */

    /* Creation du thread CMM */
    ret = pthread_create(&pthread_cmm_hnd , NULL, fn_cmm, &s_cmm );
    if (ret)
    {
        fprintf(stderr, "%s", strerror (ret));
    }

#ifdef PUSU_EMUL
    /* Creation du thread Publish Subscribe (Routing CH) */
    ret = pthread_create (&pthread_pusu_hnd , NULL, fn_pusu, &s_pusu );
    if (ret)
    {
        fprintf (stderr, "%s", strerror (ret));
    }
#endif /*PUSU_EMUL */

    /* Creation du thread action_differe */
    ret = pthread_create (&pthread_action_differe_hnd , NULL, fn_action_differe, NULL );
    if (ret)
    {
        fprintf(stderr, "%s", strerror (ret));
    }

#ifdef RRC_EMUL
    usleep(100000);
    printf("Sono nell'emulazione!\n\n" );
    scenario( NUM_SCENARIO, &s_rrc, &s_cmm );
#endif /* RRC_EMUL */

    printf("Taper [RETURN] to exit\n\n" );
    getchar() ;

    flag_not_exit = 0;
#ifdef RRC_EMUL
    close_socket(&s_rrc);
#endif /* RRC_EMUL */
    close_socket(&s_cmm);

#ifdef PUSU_EMUL
    close_socket(&s_pusu);
#endif /*PUSU_EMUL */

    /* Attente de la fin des threads. */
    pthread_join (pthread_cmm_hnd, NULL);
#ifdef PUSU_EMUL
    pthread_join (pthread_pusu_hnd, NULL);
#endif /*PUSU_EMUL */
#ifdef RRC_EMUL
    pthread_join (pthread_rrc_hnd, NULL);
#endif /* RRC_EMUL */
    pthread_join (pthread_action_differe_hnd, NULL);

    return 0 ;
}
