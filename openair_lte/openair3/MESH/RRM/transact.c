/*!
*******************************************************************************

\file    	transact.c

\brief   	Gestion des transactions entre les différentes entités

\author  	BURLOT Pascal

\date    	17/07/08
  
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

#include "debug.h"
#include "L3_rrc_defs.h"
#include "rrm_util.h"
#include "transact.h"


//! Selection locale du mode de debug
#define DBG_TRANSACT 0

#if DBG_TRANSACT==0
//! Macro inactive
#define PRINT_TRANSACT(...) 
#else
//! Macro affichant  la liste de transaction
#define PRINT_TRANSACT(...) print_transact( __VA_ARGS__ )
#endif 

/*!
*******************************************************************************
\brief 	La fonction affiche a l'ecran la liste (pour DEBUG )
		
\return  aucune valeur retournee
*/
static void print_transact( 
	transaction_t *pEntry  ///< pointeur sur la liste de transaction
	)
{
	transaction_t *pCurrentItem = pEntry;
	
	fprintf(stderr,"transaction_t=[\n");
	while ( pCurrentItem != NULL)
	{ 
		fprintf(stderr,"  @%p(.id=%u, .interf_id=%u,  .funct_id=%u, .ttl=%u,  .next=%p, .parent_id=%d, .parent_status=%d)\n",
				pCurrentItem, pCurrentItem->id, pCurrentItem->interf_id,pCurrentItem->funct_id,
				pCurrentItem->ttl, pCurrentItem->next, pCurrentItem->parent_id, pCurrentItem->parent_status);
		
		pCurrentItem = pCurrentItem->next ;
	}
	fprintf(stderr," ]\n");
}

/*!
*******************************************************************************
\brief 	La fonction ajoute un element en debut de la liste des transactions 
		non-cloturees.
		
\return  retourne le pointeur de debut de liste.
*/
transaction_t *add_item_transact( 
					transaction_t **pEntry 	, ///< pointeur sur l'entree sur la liste de transaction 
					unsigned int   id		, ///< ID de la transaction
					unsigned int   interf_id, ///< Identification de l'interface de la transaction
					unsigned int   funct_id , ///< Identification de la function a l'origine de la transaction
					unsigned int   parent	, ///< Transaction parent
					unsigned int   status     ///< Status de la transaction
					)
{
	transaction_t *pOldEntry = *pEntry;
	
	transaction_t *pNewItem = RRM_MALLOC(transaction_t , 1 ) ;
	
	PNULL(pNewItem) ;	
	if ( pNewItem == NULL ) 
		return NULL ;
		
	*pEntry	    		= pNewItem 			;
	pNewItem->next 		= pOldEntry	 		;
	pNewItem->id  		= id 		 		;
	pNewItem->interf_id	= interf_id	 		;
	pNewItem->funct_id	= funct_id	 		;
	pNewItem->parent_id	= parent		 	;
	pNewItem->parent_status	= status		 	;
	pNewItem->ttl  		= TTL_DEFAULT_VALUE ;
	
	PRINT_TRANSACT( *pEntry );
	return pNewItem ;
}

/*!
*******************************************************************************
\brief 	La fonction retire un element de la liste des transactions  non-cloturees.
		
\return  aucune valeur.
*/
void del_item_transact( 
	transaction_t **pEntry , ///< pointeur sur l'entree sur la liste de transaction 
	unsigned int   id 		 ///< ID de la transaction a detruire
	)
{
	transaction_t *pCurrentItem = *pEntry;
	transaction_t *pNextItem	;
	transaction_t **ppPrevItem  = pEntry ;
	
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
	
	PRINT_TRANSACT( *pEntry );
}

/*!
*******************************************************************************
\brief 	La fonction recherche un element dans  la liste des transactions 
		non-cloturees.
		
\return  la valeur retournee est la transaction trouvee sinon NULL
*/
transaction_t *get_item_transact( 
					transaction_t *pEntry , ///< pointeur sur l'entree sur la liste de transaction
					unsigned int   id 		///< ID de la transaction a rechercher
					)
{
	transaction_t *pCurrentItem = pEntry;
	
	while ( (pCurrentItem != NULL) && ( pCurrentItem->id != id ))
	{ 
		pCurrentItem = pCurrentItem->next ;
	}	
	return pCurrentItem ;
}

/*!
*******************************************************************************
\brief 	La fonction rearme la valeur du ttl de la transaction
	
\return  aucune valeur retournee
*/
void set_ttl_transact( 
	transaction_t *pEntry 	,  ///< pointeur sur l'entree sur la liste de transaction
	unsigned int   id 		,  ///< ID de la transaction
	unsigned int value 		   ///< valeur du ttl
	)
{
	transaction_t *transaction = get_item_transact( pEntry , id ) ;
	if ( transaction != NULL )
		transaction->ttl = value ;
}

/*!
*******************************************************************************
\brief 	La fonction decremente les valeurs du ttl de toutes les transactions
		    
\return  aucune valeur retournee
*/
void dec_all_ttl_transact( 
	transaction_t *pEntry ///< pointeur sur l'entree sur la liste de transaction
	)
{
	transaction_t *pCurrentItem = pEntry;
	
	while ( pCurrentItem != NULL)
	{ 
		if (pCurrentItem->ttl !=0 )
			pCurrentItem->ttl -=1 ;
		pCurrentItem = pCurrentItem->next ;
	}	
}

/*!
*******************************************************************************
\brief 	La fonction supprime les transactions dont le ttl est nulle
	    
\return  aucune valeur retournee
*/
void del_all_obseleted_transact( 
	transaction_t **pEntry ///< pointeur sur l'entree sur la liste de transaction
	)
{
	transaction_t *pCurrentItem = *pEntry;
	transaction_t *pNextItem ;
	
	while ( pCurrentItem != NULL)
	{ 
		pNextItem = pCurrentItem->next ;
		
		if (pCurrentItem->ttl == 0 )
		{
			fprintf(stderr,"Delete obselete Transaction: \n" ) ;
			print_transact(pCurrentItem) ;
			del_item_transact( pEntry ,  pCurrentItem->id ) ;
		}			
			
		pCurrentItem = pNextItem ;
	}	
}
