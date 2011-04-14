/*
                               bypass_phy.c
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include "SIMULATION/PHY_EMULATION/TRANSPORT/multicast_link.h"
#include "SIMULATION/PHY_EMULATION/TRANSPORT/defs.h"
//#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/extern.h"
//#include <rtai_fifos.h>
//extern pthread_mutex_t Bypass_phy_wr_mutex;
//extern pthread_cond_t Bypass_phy_wr_cond;
//extern char Bypass_phy_wr;

#define MAC_BYPASS_PHY_C
#include "bypass_phy.h"
//#include "bypass_session_layer_defs.h"
#define msg printf
//-----------------------------------------------------------------------------
void
bypass_phy_fifo_open (void)
{
//-----------------------------------------------------------------------------
  while ((fifo_bypass_phy_kern2user = open ("/dev/rtf52", O_RDONLY)) < 0) {
    printf ("[BYPASS PHY][INIT] open fifo  /dev/rtf28 returned %d\n", fifo_bypass_phy_kern2user);
    sleep (1);
  }

  while ((fifo_bypass_phy_user2kern = open ("/dev/rtf29", O_WRONLY | O_NONBLOCK))<0){// | O_NDELAY)) < 0) {
    printf ("[BYPASS PHY][INIT] open fifo  /dev/rtf29 returned %d\n", fifo_bypass_phy_user2kern);
    sleep (1);
  }

  while ((fifo_bypass_phy_kern2user_control = open ("/dev/rtf32", O_RDONLY)) < 0) {
    printf ("[BYPASS PHY][INIT] open fifo  /dev/rtf32 returned %d\n", fifo_bypass_phy_kern2user_control);
    sleep (1);
  }
  
  while ((fifo_bypass_mac = open ("/dev/rtf33", O_WRONLY | O_NONBLOCK )) < 0){// | O_NDELAY)) < 0) {
    printf ("[BYPASS PHY][INIT] open fifo  /dev/rtf33 returned %d\n", fifo_bypass_mac);
    sleep (1);
  }

  while ((fifo_mac_bypass= open ("/dev/rtf34", O_RDONLY)) < 0) {
    printf ("[BYPASS PHY][INIT] open fifo  /dev/rtf34 returned %d\n", fifo_mac_bypass);
    sleep (1);
  }
  printf ("[BYPASS PHY][INIT] fifo /dev/rtf52(%d)  opened\n",fifo_bypass_phy_kern2user);
  printf ("[BYPASS PHY][INIT] fifo /dev/rtf29(%d)  opened\n",fifo_bypass_phy_user2kern);
  printf ("[BYPASS PHY][INIT] fifo /dev/rtf32(%d)  opened\n",fifo_bypass_phy_kern2user_control);
 printf ("[BYPASS PHY][INIT] fifo /dev/rtf33(%d)  opened\n",fifo_bypass_mac);
  printf ("[BYPASS PHY][INIT] fifo /dev/rtf34(%d)  opened\n",fifo_mac_bypass);
}

//-----------------------------------------------------------------------------
int
bypass_phy_fifo_write (int num_bytesP, char *rx_bufferP)
{
  //-----------------------------------------------------------------------------
  
  //  printf("FIFO WRITE START, Num_bytes %d, rx_buffer %p \n",num_bytesP,rx_bufferP);
  int             bytes_written;
  char ff=1;

  if (num_bytesP > 0) {
    bytes_written = read (fifo_mac_bypass,&ff, 1); //attend le signal du mac
//                  printf("[Bypass_fifo_write] read MAC control %d bytes\n",bytes_written);

      bytes_written = write (fifo_bypass_phy_user2kern, rx_bufferP, num_bytesP); //ecrit le paquet pour le bypass RX
  //      printf("[Bypass_fifo_write] write %d/%d bytes\n",bytes_written,num_bytesP);
      if(num_bytesP!=bytes_written)  printf("WARRRRNNNNIIIIINGCCCCCCCCCCCCC\n");
        //    bytes_written = read (fifo_bypass_phy_kern2user_control, rx_bufferP, 1);//attend le signal du bypass rx (bloque la lecture d'une nouvelle socket)
    //printf("[Bypass_fifo_write] read BYPASS control %d bytes\n",bytes_written);
    // bytes_written = write (fifo_bypass_mac,&ff,1); //ecrit le paquet pour le bypass RX
    //printf("[Bypass_fifo_write] write MAC CONTROL %d/%d bytes\n",bytes_written,num_bytesP);
   
    
  }
}
//-----------------------------------------------------------------------------
void           *
bypass_phy_fifo_read (void *arg)
{
  //-----------------------------------------------------------------------------
  int             bytes_header_read;
  int             total_bytes_read;
  int             bytes_read;
  int             bytes_sent;
  bypass_proto2multicast_header_t *header;
  int jj;

  printf ("[BYPASS PHY][THREAD_FIFO_R] STARTED\n");
   do {
    
    header = (bypass_proto2multicast_header_t *) kern2user_data;
    bytes_header_read = 0;
    do {

      bytes_read = read (fifo_bypass_phy_kern2user, &kern2user_data[bytes_header_read], sizeof (bypass_proto2multicast_header_t) - bytes_header_read);
      //  msg("[BYPASS_PHY][THREAD_FIFO_READ] bytes_read=%d\n",bytes_read);
      if (bytes_read >= 0) {
        bytes_header_read += bytes_read;
      } else {
        msg ("[BYPASS PHY][THREAD_FIFO_R] FIFO MAC->MULTICAST\n");
        exit (-3);
      }
    } while (bytes_header_read != sizeof (bypass_proto2multicast_header_t));
    //msg("[BYPASS_PHY][THREAD_FIFO_READ] READ_HEADER OKK\n");
    //pthread_mutex_lock(&Rx_tx_mutex);   
//printf("TX_take mutex\n");    
    total_bytes_read = 0;
    if (header->size > 0) {
      // msg("[BYPASS_PHY][THREAD_FIFO_READ] HEADER SIZE OKK\n");
      do {
	if(header->size-total_bytes_read<=1000)
	  bytes_read = read (fifo_bypass_phy_kern2user, &kern2user_data[total_bytes_read + sizeof (bypass_proto2multicast_header_t)],header->size - total_bytes_read);
	else
	  bytes_read = read (fifo_bypass_phy_kern2user, &kern2user_data[total_bytes_read + sizeof (bypass_proto2multicast_header_t)],1000);
	//msg("[BYPASS_PHY][THREAD_FIFO_READ] 2ND READ bytes=%d\n",bytes_read);
        if (bytes_read >= 0) {
          total_bytes_read += bytes_read;
        } else {
          msg ("[BYPASS PHY][THREAD_FIFO_R] FIFO MAC->MULTICAST\n");
        }
      } while (total_bytes_read != header->size);
    } else {
       printf ("[BYPASS PHY][THREAD_FIFO_R] WARNING NO PAYLOAD\n");
    }
    
    
  //  while(!Rx_tx_var)
  // pthread_cond_wait(&Rx_tx_cond,&Rx_tx_mutex);

    bytes_sent = multicast_link_write_sock (0, &kern2user_data[0], total_bytes_read + sizeof (bypass_proto2multicast_header_t));
    if (bytes_sent != (total_bytes_read + sizeof (bypass_proto2multicast_header_t))) {
      printf("[BYPASS PHY][THREAD_FIFO_R] ERROR multicast_link_write_sock returned %d instead of %d", bytes_sent, total_bytes_read + sizeof (bypass_proto2multicast_header_t));
    }
    //          printf("[BYPASS PHY][THREAD_FIFO_R] multicast_link_write_sock %d bytes\n", bytes_sent);
    // Rx_tx_var=1;
      //    pthread_mutex_unlock(&Rx_tx_mutex);
    //printf("TX FREE mutex\n");
    //pthread_cond_signal(&Rx_tx_cond);
    
  } while (1);

  return arg;
}



//-----------------------------------------------------------------------------
void
bypass_phy_start_message_relay_threads (void)
{
  //-----------------------------------------------------------------------------
  if (pthread_create (&thread_fifo_bypass_phy_read, NULL, bypass_phy_fifo_read, NULL) != 0) {
    perror ("[BYPASS PHY] Thread relay LOCAL MAC -> REMOTE MACs creation");
    exit (-2);
  } else {
    pthread_detach (thread_fifo_bypass_phy_read);       // disassociate from parent
  }
}

//-----------------------------------------------------------------------------
void
bypass_phy_init (void)
{
  //-----------------------------------------------------------------------------

  bypass_phy_fifo_open ();
  bypass_phy_start_message_relay_threads ();
  multicast_link_start (bypass_phy_fifo_write);
}


int
main (int argc, char **argv)
{
  char            ch;
  int             key_pressed;

  bypass_phy_init ();
  while (1) {
    //do not consume cpu
    key_pressed = read (STDIN_FILENO, &ch, 1);
    printf("EXIT\n");
  }
}
