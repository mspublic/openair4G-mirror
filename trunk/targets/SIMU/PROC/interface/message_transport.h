

#include "message_modules.h"
#include "object_reference.h"

#define SEND_MESSAGE(to,from,mess) do {sendMessage(to, from, mess);} while(0);

extern void sendMessage(const obj_ref_t* toP, const obj_ref_t* fromP, const msg_t2 messageP);
