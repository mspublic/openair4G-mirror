#define COMPONENT_MESSAGE_TRANSPORT

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message_transport.h"
#include "message_modules.h"
#include "object_reference.h"

void sendMessage (const obj_ref_t* toP,  const obj_ref_t* fromP, const msg_t2 messageP)
{

  (*toP->callback)(toP, fromP, messageP);
  
}
