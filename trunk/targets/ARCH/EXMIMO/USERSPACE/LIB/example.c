#include <stdio.h>
#include "openair0_lib.h"

exmimo_config_t *p_exmimo_config;
exmimo_id_t     *p_exmimo_id;

unsigned int build_rflocal(txi, txq, rxi, rxq)
{
    return (txi + txq<<6 + rxi<<12 + rxq<<18);
}
unsigned int build_rfdc(int dcoff_i_rxfe, int dcoff_q_rxfe)
{
    return (dcoff_i_rxfe + dcoff_q_rxfe<<8);
}

void test_config(int card, int ant, unsigned int rf_mode)
{
    p_exmimo_config->framing.eNB_flag   = 0;
    p_exmimo_config->framing.tdd_config = 0;
    
    p_exmimo_config->rf.rf_freq_rx[ant] = 1907600000;
    p_exmimo_config->rf.rf_freq_tx[ant] = 1907600000;;
    p_exmimo_config->rf.rx_gain[ant][0] = 20;
    p_exmimo_config->rf.tx_gain[ant][0] = 10;
    p_exmimo_config->rf.rf_mode[ant] = rf_mode;
    
    p_exmimo_config->rf.rf_local[ant] = build_rflocal(20,25,26,04);
    p_exmimo_config->rf.rf_rxdc[ant] = build_rfdc(128, 128);
    p_exmimo_config->rf.rf_vcocal[ant] = 0xE<<6 + 0xE;

    openair0_dump_config( card );
}

void main(void)
{
    int ret, card, ant;
    unsigned int my_rf_mode;
    
    unsigned int *p_rx_ant0, *p_tx_ant0;
    
    
    ret = openair0_open();
    
    if ( ret != 0 )
    {
        if (ret == -1)
            printf("Error opening /dev/openair0");
        if (ret == -2)
            printf("Error mapping bigshm");
        if (ret == -3)
            printf("Error mapping RX or TX buffer");
        return;
    }
    
    my_rf_mode =  RXEN + TXEN + TXLPFNORM + TXLPFEN + TXLPF25 + RXLPFNORM + RXLPFEN + RXLPF25 + LNA1ON +LNAMax + RFBBNORM;
    my_rf_mode += DMAMODE_RX + DMAMODE_TX;
    
    printf ("Detected %d number of cards.\n", openair0_num_detected_cards);
   
    card =0;
    ant  =0;
    
    printf ("Will configure card %d, antenna %d\n", card, ant);

    p_exmimo_config = openair0_exmimo_pci[card].exmimo_config_ptr;
    p_exmimo_id     = openair0_exmimo_pci[card].exmimo_id_ptr;
        
    printf("Card %d: ExpressMIMO %d, HW Rev %d, SW Rev 0x%d\n", card, p_exmimo_id->board_exmimoversion, p_exmimo_id->board_hwrev, p_exmimo_id->board_swrev);

    test_config( card, ant, my_rf_mode);
    
    // pointer to data
    
    p_rx_ant0 = openair0_exmimo_pci[ card ].adc_head[ ant ];
    p_tx_ant0 = openair0_exmimo_pci[ card ].dac_head[ ant ];
    
    // do something with the data here
    
    
    openair0_close();
}
