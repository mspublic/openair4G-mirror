#define MESSAGE_MODULES_C

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "message_modules.h"


msg_t2 openairDeserializeMsg(const char* char_bufferP, const int sizeP)
{
  msg_t2 message;
  if (sizeP >= sizeof(msg_head_t)) {
    memcpy(&message.head, char_bufferP, sizeof (msg_head_t));
    if (sizeP > sizeof(msg_head_t)) {
      message.head.size         = sizeP - sizeof (msg_head_t);
      message.data              = malloc(sizeP - sizeof (msg_head_t));
      memcpy(message.data, &char_bufferP[sizeof (msg_head_t)], sizeP - sizeof (msg_head_t));
    } else {
      message.head.size         = 0;
      message.data              = NULL;
    }
  } else {
    message.head.msg_type         = MSG_UNDEFINED;
    message.head.msg_sub_type     = SUBMSG_UNDEFINED;
    message.head.size             = 0;
    message.data                  = NULL;
  }
  return message;
}

msg_t2 Execute_Msg(int frame,int slot) {
  msg_t2 msg;
  Exec_Msg_t msg_CO;
  memset(&msg_CO, 0, sizeof(msg_CO));
  msg_CO.frame=frame;
  msg_CO.slot=slot;
  msg.head.msg_type             = Exec_Msg;
  msg.head.msg_sub_type         = HELLO;
  msg.data                      = (char*)malloc(sizeof(msg_CO)+1);
  assert(msg.data != NULL);
  msg.head.size = sizeof(msg_CO)+1;
  memcpy (msg.data, &msg_CO, sizeof(msg_CO)+1);
  return msg;
}


msg_t2 Execute_Msg_Response(){
  msg_t2 msg;
  Exec_Msg_Response_t msg_CO;
  memset(&msg_CO, 0, sizeof(msg_CO));
  msg_CO.frame=0;
  msg_CO.slot=0;
  msg.head.msg_type             = Exec_Msg_Response;
  msg.head.msg_sub_type         = HELLO;
  msg.data                      = (char*)malloc(sizeof(msg_CO)+1);
  assert(msg.data != NULL);
  msg.head.size = sizeof(msg_CO)+1;
  memcpy (msg.data, &msg_CO, sizeof(msg_CO)+1);
  return msg;
}


