/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
    included in this distribution in the file called "COPYING". If not, 
    see <http://www.gnu.org/licenses/>.

   Contact Information
   OpenAirInterface Admin: openair_admin@eurecom.fr
   OpenAirInterface Tech : openair_tech@eurecom.fr
   OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
   Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
 
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

void read_firmware_buffer(int card)
{
    int i;
    unsigned int *p= (unsigned int *) (openair0_exmimo_pci[card].firmware_block_ptr);
    printf("firmware_buffer: \n");
    
    for (i=0; i<0x30; i++)
        printf("u32 fwbuf[%d]: value=%08X\n", i, p[i]);
    
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


    //read_firmware_buffer(card);


    test_config( card, ant, my_rf_mode);
    
    // pointer to data
    
    p_rx_ant0 = openair0_exmimo_pci[ card ].adc_head[ ant ];
    p_tx_ant0 = openair0_exmimo_pci[ card ].dac_head[ ant ];
    
    // do something with the data here
    
    
    openair0_close();
}
