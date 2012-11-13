//WARNING: Do not include this header directly. Use intertask_interface.h instead.

#if defined(MESSAGE_DEFINITION)
MESSAGE_DEF(SctpNewS1APDataReq sctpNewS1APDataReq;)
MESSAGE_DEF(SctpS1APInit       sctpS1APInit;)
MESSAGE_DEF(SctpS6APInit       sctpS6APInit;)
#else
# ifndef SCTP_MESSAGES_DEF_H_
# define SCTP_MESSAGES_DEF_H_
typedef struct {
    uint8_t  *buffer;
    uint32_t  bufLen;
    uint32_t  assocId;
    uint8_t   stream;
} SctpNewS1APDataReq;

typedef struct {
    uint16_t  port;
    char     *address;
} SctpS1APInit, SctpS6APInit;
# endif /* SCTP_MESSAGES_DEF_H_ */
#endif
