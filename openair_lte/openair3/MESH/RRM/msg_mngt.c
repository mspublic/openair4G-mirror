/*!
*******************************************************************************

\file       msg_mngt.c

\brief      Gestion de l'envoi et la reception de message.

\author     BURLOT Pascal

\date       17/07/08
  
\par     Historique:
            $Author$  $Date$  $Revision$
            $Id$
            $Log$

*******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <pthread.h>

#include "debug.h"
#include "L3_rrc_defs.h"
#include "rrm_util.h"
#include "rrm_sock.h"
#include "msg_mngt.h"

//! Selection locale du mode de debug
#define DBG_MSG_MNGT 0

#if DBG_MSG_MNGT==0
//! Macro inactive
#define PRINT_MSG_MNGT(...) 
#else
//! Macro affichant  la file de message
#define PRINT_MSG_MNGT(...) print_msg_mngt( __VA_ARGS__ )
#endif 

/*!
*******************************************************************************
\brief  La fonction affiche a l'ecran la liste des messages
        
\return  aucune valeur retournee
*/
static void print_msg_mngt( file_msg_t *pEntry , int id )
{
    file_msg_t *pCurrentItem = pEntry;
    
    fprintf(stderr,"file_msg_t(%d)=[\n", id);
    while ( pCurrentItem != NULL)
    { 
        fprintf(stderr,"  @%p(.s=%p, .msg=%p, .next=%p)\n", pCurrentItem, pCurrentItem->s, 
                    pCurrentItem->msg, pCurrentItem->next);
        pCurrentItem = pCurrentItem->next ;
    }
    fprintf(stderr," ]\n");
}

/*!
*******************************************************************************
\brief  La fonction initialise  la file des messages.
        
\return aucune valeur.
*/
void init_file_msg( 
    file_head_t *file_hd , ///< descripteur de la file des messages
    int  id                ///< file id
    ) 
{
    pthread_mutex_init( &( file_hd->mutex ), NULL ) ;

    file_hd->id   = id;
    file_hd->file = NULL ;
}

/*!
*******************************************************************************
\brief  La fonction ajoute un element en debut de la file des messages.

\return  retourne le pointeur le nouvel element de la liste.
*/
file_msg_t *put_msg( 
    file_head_t *file_hd ,  ///< descripteur de la file des messages
    sock_rrm_t *s,          ///< socket associe au message
    msg_t *msg              ///< le message a ajouter dans la liste
    ) 
{
    file_msg_t *pOldEntry = file_hd->file;
    
    file_msg_t *pNewItem = RRM_MALLOC( file_msg_t , 1 ) ;
        
    if ( pNewItem == NULL ) 
        return NULL ;

    CALL(pthread_mutex_lock( &(file_hd->mutex) ));
        
    file_hd->file       = pNewItem          ;
    pNewItem->next      = pOldEntry         ;
    pNewItem->s         = s                 ;
    pNewItem->msg       = msg               ;
    
    PRINT_MSG_MNGT( file_hd->file , file_hd->id );
    
    CALL(pthread_mutex_unlock( &(file_hd->mutex) ));

    return pNewItem ;
}

/*!
*******************************************************************************
\brief   La fonction retire le dernier element de la file des messages.

\return  retourne le pointeur sur l'element enleve de la liste.
*/
file_msg_t *get_msg( 
    file_head_t *file_hd ///< descripteur de la file des messages
    ) 
{
    file_msg_t *pCurrentItem = file_hd->file ;
    file_msg_t *pNextItem   ;
    file_msg_t **ppPrevItem  = &(file_hd->file) ;
        
    if ( (pCurrentItem == NULL)  ) 
        return NULL ;

    CALL(pthread_mutex_lock( &(file_hd->mutex) ));

    while ( pCurrentItem != NULL )
    { 
        pNextItem = pCurrentItem->next ;
        if ( pNextItem == NULL )
        { /* Dernier element */
            *ppPrevItem = NULL ;
            break ;
        }
        ppPrevItem   = &(pCurrentItem->next) ;
        pCurrentItem = pNextItem ;
    }
    
    PRINT_MSG_MNGT( file_hd->file , file_hd->id );
    PRINT_MSG_MNGT( pCurrentItem , file_hd->id *-1 );
    
    CALL(pthread_mutex_unlock( &(file_hd->mutex) ));

    return pCurrentItem ; 
}

