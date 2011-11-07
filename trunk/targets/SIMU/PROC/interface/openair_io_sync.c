
#define OPENAIR_IO_SYNC_C
#define OPENAIR_IO

#include "openair_io_sync.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "definitions.h"
#include "../interface.h"
int mycount2=0;
void
openairIoSyncInit(openair_io_sync_t* io_syncP)
{
	io_syncP->hfd             = 0;
	io_syncP->io_sync_entries = NULL;
	io_syncP->thread_started  = 0;
}

int
openairSelect(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval *timeout)
{
  return select(nfds, readfds, writefds, exceptfds, timeout);

}


void
openairAddIoFileDesc(openair_io_sync_t* io_syncP,
		     int fd_readP,
		     int fd_writeP,
		     void (*pf) (int, obj_ref_t*),
		     obj_ref_t obj)
{
  openair_io_sync_entry_t *new_entry;

  if (((fd_readP == 0) && (fd_writeP == 0)) || (pf == NULL)) {
    //OPENAIR_ERROR("socket entry - not registering...\n");
    return;
  }
  //OPENAIR_INFO("Adding OLSR socket entry %d \n", fd);
  
  new_entry = malloc(sizeof(openair_io_sync_entry_t));
  
  new_entry->fd_read               = fd_readP;
  new_entry->fd_write              = fd_writeP;
  new_entry->process_recv_function = pf;
  new_entry->object                = obj;
  
  if (fd_readP != 0) {
    /* put in the queue */
    new_entry->next = io_syncP->io_sync_entries;
    io_syncP->io_sync_entries = new_entry;
    
    if (fd_readP + 1 > io_syncP->hfd)
      io_syncP->hfd = fd_readP + 1;
  }
}


int
openairRemoveIoFileDesc(openair_io_sync_t* io_syncP, int fd_readP, int fd_writeP, void (*pf) (int fd, obj_ref_t* objP))
{
  openair_io_sync_entry_t *entry, *prev_entry;
  
  if ((fd_readP == 0) || (pf == NULL)) {
    printf("openairRemoveIoFileDesc - not processing...\n");
    return 0;
  }
  printf("openairRemoveIoFileDesc removing openair socket entry %d\n", fd_readP);

  entry = io_syncP->io_sync_entries;
  prev_entry = NULL;
  
  while (entry) {
    if ((entry->fd_read == fd_readP) && (entry->process_recv_function == pf)) {
      // Close file descriptors
      if (fd_readP >= 0) {
	close(fd_readP);
    	}
      if ((entry->fd_write == fd_writeP) && (entry->fd_write >= 0) && (fd_writeP != fd_readP)){
	close(fd_writeP);
      }
      if (prev_entry == NULL) {
	io_syncP->io_sync_entries = entry->next;
	free((void*)entry);
	//openairFree((void*)entry);
      } else {
	prev_entry->next = entry->next;
	//openairFree((void*)entry);
	free((void*)entry);

      }
      
      if (io_syncP->hfd == fd_readP + 1) {
        /* Re-calculate highest FD */
        entry = io_syncP->io_sync_entries;
        io_syncP->hfd = 0;
        while (entry) {
          if ((entry->fd_read + 1) > io_syncP->hfd)
	    io_syncP->hfd = entry->fd_read + 1;
          entry = entry->next;
        }
      }
      return 1;
    }
    prev_entry = entry;
    entry = entry->next;
  }
  
  return 0;
}

void
openairPollIoFileDesc(openair_io_sync_t* io_syncP)
{
  int n;
  openair_io_sync_entry_t *openair_io_sync_entry;
  fd_set read_ibits;
  struct timeval tvp = { 0, 0 };
  
  /* If there are no registered sockets we
   * do not call select(2)
   */

  if (io_syncP->hfd == 0)
    return;
  
  FD_ZERO(&read_ibits);

  /* Adding file-descriptors to FD set */
  
  for (openair_io_sync_entry = io_syncP->io_sync_entries; openair_io_sync_entry; openair_io_sync_entry = openair_io_sync_entry->next) {
    FD_SET(openair_io_sync_entry->fd_read, &read_ibits);
  }

  
  /* Runnig select on the FD set */
   n = openairSelect(io_syncP->hfd, &read_ibits, NULL, NULL, &tvp);
   
   if (n == 0)
    return;
   /* Did somethig go wrong? */
   if (n < 0) {
     if (errno != EINTR) {
       const char *const err_msg = strerror(errno);
       //OPENAIR_ERROR("select: %s", err_msg);
     }
     return;
   }
   
   for (openair_io_sync_entry = io_syncP->io_sync_entries; openair_io_sync_entry; openair_io_sync_entry = openair_io_sync_entry->next) {
     //OPENAIR_TRACE("polling sock fd %d",openair_sockets->fd);
     if (FD_ISSET(openair_io_sync_entry->fd_read, &read_ibits))
       openair_io_sync_entry->process_recv_function(openair_io_sync_entry->fd_read,&openair_io_sync_entry->object);
   }
}


void*
openairIoSyncThread (void *param)
{
  int n;
  openair_io_sync_t*        io_sync = (openair_io_sync_t*)param;
  openair_io_sync_entry_t* openair_io_sync_entry;
  fd_set read_ibits;
  fd_set write_ibits;
  struct timeval tvp = { 0, 0 };
  
 // printf("openairIoSyncThread started\n");
  while (io_sync != NULL) {
    
    /* If there are no registered sockets we
     * do not call select(2)
     */

    if (io_sync->hfd != 0) {

      //printf("openairIoSyncThread loop\n");
      FD_ZERO(&read_ibits);
      
      /* Adding file-descriptors to FD set */
      
      for (openair_io_sync_entry = io_sync->io_sync_entries; openair_io_sync_entry; openair_io_sync_entry = openair_io_sync_entry->next) {
	FD_SET(openair_io_sync_entry->fd_read, &read_ibits);
      }
      
      
      /* Runnig select on the FD set */
      //n = openairSelect(io_sync->hfd, &read_ibits, NULL, NULL, &tvp);
      n = openairSelect(io_sync->hfd, &read_ibits, NULL, NULL, NULL);
      
      if (n != 0) {
	/* Did somethig go wrong? */
	if (n < 0) {
	  if (errno != EINTR) {
	    const char *const err_msg = strerror(errno);
	    //OPENAIR_ERROR("select: %s", err_msg);
	  }
	} else {
	  for (openair_io_sync_entry = io_sync->io_sync_entries; openair_io_sync_entry; openair_io_sync_entry = openair_io_sync_entry->next) {
	    //OPENAIR_TRACE("polling sock fd %d",openair_sockets->fd);
	    if (FD_ISSET(openair_io_sync_entry->fd_read, &read_ibits))
	      openair_io_sync_entry->process_recv_function(openair_io_sync_entry->fd_read,&openair_io_sync_entry->object);
	  }
	}
      }
    }
  }
  return NULL;
}

int
trig_wait (void *param)
{
  int n;
  openair_io_sync_t*        io_sync = (openair_io_sync_t*)param;
  openair_io_sync_entry_t* openair_io_sync_entry;
  fd_set read_ibits;
  fd_set write_ibits;
  struct timeval tvp = { 0, 0 };

    if (io_sync->hfd != 0) {

      FD_ZERO(&read_ibits);

      for (openair_io_sync_entry = io_sync->io_sync_entries; openair_io_sync_entry; openair_io_sync_entry = openair_io_sync_entry->next) {
	FD_SET(openair_io_sync_entry->fd_read, &read_ibits);
      }

      n = openairSelect(io_sync->hfd, &read_ibits, NULL, NULL, NULL);

      if (n != 0) {
	/* Did somethig go wrong? */
	if (n < 0) {
	  if (errno != EINTR) {
	    const char *const err_msg = strerror(errno);
	  }
	} else {
	  for (openair_io_sync_entry = io_sync->io_sync_entries; openair_io_sync_entry; openair_io_sync_entry = openair_io_sync_entry->next) {
	    if (FD_ISSET(openair_io_sync_entry->fd_read, &read_ibits))
	      openair_io_sync_entry->process_recv_function(openair_io_sync_entry->fd_read,&openair_io_sync_entry->object);
	  }
	}
      }
    }

  return n;
}

void wait_4Msg()
{
	Soc_t* this  = (Soc_t*)(obj_inst[0].ptr->mem_ref.pointer);
	trig_wait((void*)&this->m_io_sync);
}

void wait_4slot(int *slot,int *frame)
{
	Soc_t* this  = (Soc_t*)(obj_inst[0].ptr->mem_ref.pointer);
    s_t* st2=(s_t*)(Instance[1].gm->mem_ref.pointer);
	trig_wait((void*)&this->m_io_sync);
	*slot=st2->slot;
	*frame=st2->frame;
}

int
channel_wait (int sock)
{
  int n;
  fd_set read_ibits;
  fd_set write_ibits;
  struct timeval tvp = { 0, 0 };

      FD_ZERO(&read_ibits);

      FD_SET(sock,&read_ibits);

      n = openairSelect(sock+1,&read_ibits, NULL, NULL, NULL);

      if (n != 0) {
	if (n < 0) {
	  if (errno != EINTR) {
	    const char *const err_msg = strerror(errno);
	  }
	} else {
	    FD_ISSET(sock, &read_ibits);
	  }

    return n;
      }
}


void
openairIoSyncCreateThread(openair_io_sync_t* io_syncP)
{
  if (io_syncP->thread_started == 0) {
    if (pthread_create (&io_syncP->thread, NULL, openairIoSyncThread, (void*)io_syncP) != 0) {
	  //  if (openairIoSyncThread((void*)io_syncP) != 0) {

  } else {
      io_syncP->thread_started = 1;
    }
  }
}

