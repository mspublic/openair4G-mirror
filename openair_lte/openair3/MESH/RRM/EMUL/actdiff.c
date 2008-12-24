/*!
*******************************************************************************

\file       actdiff.c

\brief      Gestion des actions differees d'envoi de messages sur les sockets
            ( emulation du traitement par un retard )


\author     BURLOT Pascal

\date       13/08/08

   
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
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/un.h>

#include "debug.h"
#include "L3_rrc_defs.h"
#include "rrm_util.h"
#include "rrm_sock.h"
#include "actdiff.h"

#define DBG_ACTDIFF 1

#if DBG_ACTDIFF==1
#define PRINT_ACTDIFF(...) 
#else
#define PRINT_ACTDIFF(...) print_actdiff( __VA_ARGS__ )
#endif 


/******************************************************************************
 * \brief   La fonction affiche a l'ecran la liste
 *      
 * \return  aucune valeur retournee
 */
static void print_actdiff( actdiff_t *pEntry )
{
    actdiff_t *pCurrentItem = pEntry;
    
    fprintf(stderr,"actdiff_t=[\n");
    while ( pCurrentItem != NULL)
    { 
        fprintf(stderr,"  @%p(.tv=%lf, .id=%d,  .sock=%p,  .msg=%p, .next=%p)\n",
                pCurrentItem, pCurrentItem->tv, pCurrentItem->id, pCurrentItem->sock,
                pCurrentItem->msg, pCurrentItem->next);
        
        pCurrentItem = pCurrentItem->next ;
    }
    fprintf(stderr," ]\n");
    
}

/******************************************************************************
 * \brief   La fonction ajoute un element en fin de la liste des actions differees
 *          non-cloturees.
 *      
 * \return  retourne le pointeur de debut de liste.
 */
actdiff_t *add_actdiff( 
                    actdiff_t **pEntry , 
                    double delai,
                    int   id,
                    sock_rrm_t   *sock,
                    msg_t  *msg
)
{
    actdiff_t *pNewItem = RRM_MALLOC(actdiff_t , 1 ) ;
    
    PNULL(pNewItem) ;   
    if ( pNewItem == NULL ) 
        return NULL ;
    
    pNewItem->id        =  id                   ;
    pNewItem->msg       =  msg                  ;
    pNewItem->sock      =  sock                 ;

    pNewItem->tv        =  get_currentclock() + delai ;
    pNewItem->next      =  NULL         ;

    if ( *pEntry == NULL )
    {
        *pEntry             =  pNewItem             ;
    }
    else
    {
        actdiff_t *pCurrentItem = *pEntry;
        actdiff_t *pNextItem    ;
        while ( pCurrentItem != NULL )
        { 
            pNextItem =  pCurrentItem->next ;
            if ( pNextItem == NULL )
            { 
                pCurrentItem->next = pNewItem ;  
                break ;
            }
            pCurrentItem = pNextItem ;
        }
    }
        
    PRINT_ACTDIFF(*pEntry) ;
    
    return pNewItem ;
}

/******************************************************************************
 * \brief   La fonction detruit l'action no Id de la liste .
 *      
 * \return  aucune valeur.
 */
void del_actdiff( actdiff_t **pEntry , int   id )
{
    actdiff_t *pCurrentItem = *pEntry;
    actdiff_t *pNextItem    ;
    actdiff_t **ppPrevItem  = pEntry ;
    
    if ( (pCurrentItem == NULL)  || (pEntry==NULL)) 
        return ;

    while ( pCurrentItem != NULL )
    { 
        pNextItem =  pCurrentItem->next ;
        if ( pCurrentItem->id == id )
        { 
            *ppPrevItem = pNextItem;  
                
            RRM_FREE( pCurrentItem ) ;
            break ;
        }
        ppPrevItem   = &(pCurrentItem->next) ;
        pCurrentItem = pNextItem ;
    }
    
    PRINT_ACTDIFF(*pEntry) ;
}


/******************************************************************************
 * \brief   La fonction traite les actions arrivees a echeance.
 *      
 * \return  aucune valeur.
 */
void processing_actdiff( actdiff_t **pEntry )
{
    double tv_current ;
    
    actdiff_t *pCurrentItem = *pEntry;
    actdiff_t *pNextItem    ;
    
    if ( (pCurrentItem == NULL)  || (pEntry==NULL)) 
        return ;

    tv_current  =  get_currentclock() ;
    
    while ( pCurrentItem != NULL )
    { 
        pNextItem =  pCurrentItem->next ;       
        if ( pCurrentItem->tv <= tv_current )
        { 
            send_msg( pCurrentItem->sock, pCurrentItem->msg ) ;
            //fprintf(stderr,"act diff: %lf \n", tv_current);
            del_actdiff(pEntry,pCurrentItem->id) ; 
        }
        pCurrentItem = pNextItem ;
    }
}



