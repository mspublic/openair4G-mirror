#ifndef __OBJECT_REFERENCE_H__
#    define __OBJECT_REFERENCE_H__


#include "message_modules.h"
#include <netinet/in.h>
#include <sys/un.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {MEMORY_REFERENCE = 0,
	          OMNETPP_REFERENCE,
	          FIFO_REFERENCE,
	          UNIX_SOCKET_REFERENCE,
	          INET_SOCKET_REFERENCE,
	          INET6_SOCKET_REFERENCE,
	          NUM_TYPES_OBJECT_REFERENCE } obj_ref_type_t;


typedef enum {RID_CLASS = 0, TC_CLASS, CMM_CLASS, LOG_CLASS, NUM_DECLARED_CLASSE_NAMES, GM_CLASS} class_name_t;
typedef struct {
	struct sockaddr_in  sockaddr;
	int                 sock_fd;
} sock_inet_reference_t;

typedef struct {
	struct sockaddr_in6 sockaddr;
	int                 sock_fd;
} sock_inet6_reference_t;

typedef struct {
    struct sockaddr_un sockaddr;
	int                 sock_fd;
} sock_un_reference_t;

typedef struct {
    void*           pointer;
} memory_reference_t;

struct obj_ref {
    class_name_t        object_class;
    obj_ref_type_t      ref_type;
    void  (*callback) (const struct obj_ref *, const struct obj_ref *, msg_t2);
    memory_reference_t  mem_ref;
    union {
        sock_inet_reference_t  sock_inet_ref;
        sock_inet6_reference_t sock_inet6_ref;
        sock_un_reference_t    sock_un_ref;
        // fifo_reference_t, etc.
    } obj_ref;
};

typedef struct obj_ref obj_ref_t;

//public_object_reference(const char* objGetClassName(const obj_ref_t objP);)

#ifdef __cplusplus
}
#endif

#endif
