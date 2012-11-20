#include <stdio.h>

#include "mme_config.h"
#include "gtpv1_u.h"

#include "intertask_interface.h"

static int gtpv1_u_send_init_udp(uint16_t port_number);

static int gtpv1_u_send_init_udp(uint16_t port_number) {
    // Create and alloc new message
    MessageDef *message_p;
    message_p = (MessageDef *)malloc(sizeof(MessageDef));

    message_p->messageId = UDP_INIT;
    message_p->originTaskId = TASK_GTPV1_U;
    message_p->destinationTaskId = TASK_UDP;
    message_p->msg.udpInit.port = port_number;
    message_p->msg.udpInit.address = "0.0.0.0"; //ANY address

    return send_msg_to_task(TASK_UDP, message_p);
}

int gtpv1_u_init(const mme_config_t *mme_config) {
    return gtpv1_u_send_init_udp(mme_config->gtpv1_u_config.port_number);
}
