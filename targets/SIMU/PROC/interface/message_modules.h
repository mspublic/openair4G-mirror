
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifndef __MESSAGE_MODULES_H__
#    define __MESSAGE_MODULES_H__

#    ifdef MESSAGE_MODULES_C
#        define private_message_modules(x) x
#        define friend_message_modules(x) x
#        define public_message_modules(x) x
#    else
#        define private_message_modules(x)
#        define friend_message_modules(x)
#        define public_message_modules(x) extern x
#    endif


  typedef enum {
    Exec_Msg=1,
    Exec_Msg_Response=2,
    MSG_UNDEFINED,
    NB_MSG                     
  } MSG_T ;
  

  typedef enum {
    DATA = 0,
    CAC_CONNECTION_SETUP,
    HELLO,
    SUBMSG_UNDEFINED,
    NB_SUB_MSG
  } MSG_SUB_T ;
  
  
  typedef enum{
    RID_BD_UPDATE,
    TC_UPFATE
  }NET_MSG_TYPE;
  
  
  typedef struct {
    unsigned char  msg_type ;     
    unsigned char  msg_sub_type ; 
    unsigned int   size;         
  } msg_head_t ;
  

  typedef struct {
    msg_head_t 	head;            
    char       *data;           
  } msg_t2 ;

  typedef struct {
   int frame;
   int slot;
   }Exec_Msg_t;

  typedef struct {
  int frame;
  int slot;
  }Exec_Msg_Response_t;


    public_message_modules(msg_t2 openairDeserializeMsg(const char* char_bufferP, const int sizeP);)

    public_message_modules(msg_t2 Execute_Msg(int frame,int slot);)

    public_message_modules(msg_t2 Execute_Msg_Response();)

#ifdef __cplusplus
    }
#endif

#endif
