#ifndef S1AP_ENB_DEFAULT_VALUES_H_
#define S1AP_ENB_DEFAULT_VALUES_H_

#define ENB_TAC (0)
#define ENB_MCC (208)
#define ENB_MNC (34)

#define ENB_NAME        "Eurecom ENB"
#define ENB_NAME_FORMAT (ENB_NAME" %u")

#define S1AP_PORT_NUMBER        (36412)
#define S1AP_SCTP_PPID          (18)

#define X2AP_PORT_NUMBER        (36422)
#define X2AP_SCTP_PPID          (27)

#define SCTP_OUT_STREAMS        (64)
#define SCTP_IN_STREAMS         (64)
#define SCTP_MAX_ATTEMPTS       (5)
#define SCTP_RECV_BUFFER_SIZE   (1024)

#endif /* S1AP_ENB_DEFAULT_VALUES_H_ */
