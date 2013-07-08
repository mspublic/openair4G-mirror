#ifndef PGM_LINK_H_
#define PGM_LINK_H_

/* Define prototypes only if enabled */
#if defined(ENABLE_PGM_TRANSPORT)

int pgm_oai_init(char *if_name);

int pgm_recv_msg(int group, uint8_t *buffer, uint32_t length);

int pgm_link_send_msg(int group, uint8_t *data, uint32_t len);

#endif

#endif /* PGM_LINK_H_ */
