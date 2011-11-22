

#ifndef __OPENAIR_IO_SYNC_H__
#define __OPENAIR_IO_SYNC_H__

#    ifdef OPENAIR_IO_SYNC_C
#        define private_io_sync(x) x
#        define friend_io_sync(x) x
#        define public_io_sync(x) x
#    else
#        ifdef OPENAIR_IO
#            define private_io_sync(x)
#            define friend_io_sync(x) extern x
#            define public_io_sync(x) extern x
#        else
#            define private_io_sync(x)
#            define friend_io_sync(x)
#            define public_io_sync(x) extern x
#        endif
#    endif

#include "object_reference.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

// FIFOs are 4K on LINUX
#define IO_SYNC_RECV_BUFFER_SIZE 40960

friend_io_sync(char io_sync_recvbuf[IO_SYNC_RECV_BUFFER_SIZE];)
friend_io_sync(char io_sync_txbuf[IO_SYNC_RECV_BUFFER_SIZE];)

typedef struct openair_io_sync_entry{

	int fd_read;
	int fd_write;

	int type;
	void (*process_recv_function) (int fd, obj_ref_t*);
	obj_ref_t    object;
	struct openair_io_sync_entry *next;

}openair_io_sync_entry_t;

typedef struct openair_io_sync{
    int                     hfd;
    pthread_t               thread;
    int                     thread_started;
	openair_io_sync_entry_t *io_sync_entries;
}openair_io_sync_t;



extern void  openairIoSyncInit(openair_io_sync_t*);
int openairSelect (int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval *timeout);
extern void  openairAddIoFileDesc   (openair_io_sync_t*, int, int, void (*)(int, obj_ref_t*), obj_ref_t);
extern int   openairRemoveIoFileDesc(openair_io_sync_t*, int, int, void (*)(int, obj_ref_t*));
extern void  openairPollIoFileDesc  (openair_io_sync_t*);
void* openairIoSyncThread(void*);
extern void  openairIoSyncCreateThread(openair_io_sync_t*);


#endif
