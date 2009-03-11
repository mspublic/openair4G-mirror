/*!
*******************************************************************************

\file       rrm_constant.c

\brief      Il contient les declarations des constantes et tables relative aux 
            fonctions du RRM (Radio Resource Management ).

\author     BURLOT Pascal

\date       17/07/08

   
\par     Historique:
            $Author$  $Date$  $Revision$
            $Id$
            $Log$

*******************************************************************************
*/

#include "L3_rrc_defs.h"

#include "rrm_constant.h"

#ifdef OPENAIR2
#include "platform_constants.h"
#else
#define BCCH_PAYLOAD_SIZE_MAX 25
#define CCCH_PAYLOAD_SIZE_MAX 30
#endif

/*! \todo a definir les valeurs par defaut */

//! \brief descripteur de canal logique en fonction de la QoS
const  LCHAN_DESC        Lchan_desc[MAX_QOS_CLASS] =
{
    //  Transport    Max    Guaranteed  Max  Delay Target  LCHAN_t
    //    Block   Transport    Bit      Bit  Class  BLER
    //    size      Blocks     Rate     Rate 
    { BCCH_PAYLOAD_SIZE_MAX, 15, 64,   128,    1,    0,   LCHAN_BCCH   }, // SRB0
    { CCCH_PAYLOAD_SIZE_MAX, 15, 64,   128,    1,    0,   LCHAN_CCCH   }, // SRB1
    {      30,       20,        64,    128,    1,    0,   LCHAN_DCCH   }, // SRB2
    {      52,       20,        64,    128,    1,    0,   LCHAN_DTCH_B }, // DTCH_B
    {      52,       20,        64,    128,    1,    0,   LCHAN_DTCH   },
    {      52,       20,        64,    128,    1,    0,   LCHAN_DTCH   },
    {      52,       20,        64,    128,    1,    0,   LCHAN_DTCH   },
    {      52,       20,        64,    128,    1,    0,   LCHAN_DTCH   },
    {      52,       20,        64,    128,    1,    0,   LCHAN_DTCH   },
    {      52,       20,        64,    128,    1,    0,   LCHAN_DTCH   },
};

//! \brief descripteur de la confiuration des mesures en fonction de la QoS
const MAC_RLC_MEAS_DESC Mac_rlc_meas_desc[MAX_QOS_CLASS] =
{
    /* 0 : QOS_CLASS = SRB0 */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    },
    /* 1 : QOS_CLASS = SRB1 */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    },
    /* 2 : QOS_CLASS = SRB2 */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    },
    /* 3 : QOS_CLASS = DTCH_B */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    },
    /* 4 : QOS_CLASS  */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    },
    /* 5 : QOS_CLASS  */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    },
    /* 6 : QOS_CLASS  */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    },
    /* 7 : QOS_CLASS  */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    },
    /* 8 : QOS_CLASS  */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    },
    /* 9 : QOS_CLASS  */
    { .Meas_trigger={.Rssi=0,.Sinr={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                     .Harq_delay=0,.Bler=0,.Spec_eff=0, .Qdepth = 0} ,   
      .Mac_avg={.Rssi_forgetting_factor=0,.Sinr_forgetting_factor=0,
                .Harq_delay_forgetting_factor=0,.Bler_forgetting_factor=0,
                .Spec_eff_forgetting_factor=0} ,      
      .bo_forgetting_factor=0, .sdu_loss_trigger=0, .Rep_amount=0, .Rep_interval=0
    }
};

//! \brief descripteur de la configuration des mesures de voisinage
const SENSING_MEAS_DESC Sensing_meas_desc =
{
//  RSSI_Threshold    RSSI_F_Factor      Rep_interval
    0,                  0,              0
};




