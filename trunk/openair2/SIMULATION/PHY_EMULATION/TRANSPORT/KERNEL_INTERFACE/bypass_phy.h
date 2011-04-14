/***************************************************************************
                          bypass_phy.h  -  description
                             -------------------
 ***************************************************************************/
#ifndef __BYPASS_PHY_H__
#    define __BYPASS_PHY_H__
//-----------------------------------------------------------------------------
#    ifdef MAC_BYPASS_PHY_C
#        define private_bypass_phy(x) x
#        define public_bypass_phy(x) x
#    else
#        define private_bypass_phy(x)
#        define public_bypass_phy(x) extern x
#    endif

//-----------------------------------------------------------------------------
#define KERN2USER_BUFFER_SIZE 15000


private_bypass_phy (pthread_t thread_fifo_bypass_phy_read);
private_bypass_phy (int fifo_bypass_phy_kern2user);
private_bypass_phy (int fifo_bypass_phy_user2kern);
private_bypass_phy (int fifo_bypass_phy_kern2user_control);
private_bypass_phy (int fifo_mac_bypass);
private_bypass_phy (int fifo_bypass_mac);
private_bypass_phy (char kern2user_data[KERN2USER_BUFFER_SIZE]);


private_bypass_phy (void bypass_phy_fifo_open (void));
private_bypass_phy (int bypass_phy_fifo_write ( int num_bytesP, char *rx_bufferP));
private_bypass_phy (void *bypass_phy_fifo_read (void *arg));
private_bypass_phy (void bypass_phy_start_message_relay_threads (void));
private_bypass_phy (void bypass_phy_init (void));

private_bypass_phy (pthread_mutex_t Rx_tx_mutex);
//private_bypass_phy (pthread_cond_t Rx_tx_cond);
//char Rx_tx_var;


#endif
