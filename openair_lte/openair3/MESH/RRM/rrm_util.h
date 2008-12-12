/*!
*******************************************************************************

\file    	rrm_util.h

\brief   	Fichier d'entete contenant les declarations des types, des defines ,
			et des fonctions relatives a des routines utilitaires du RRM .

\author  	BURLOT Pascal

\date    	17/07/08

   
\par     Historique:
			$Author$  $Date$  $Revision$
			$Id$
			$Log$

*******************************************************************************
*/

#ifndef RRM_UTIL_H
#define RRM_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#if 1
//! Macro permettant l'affichage pour le debug 
	#define msg( ... ) printf(__VA_ARGS__)
//! Macro permettant l'affichage pour le debug 
	#define msg_fct( ... ) printf(__VA_ARGS__)
#else
//! Macro inactive 
	#define msg( ... ) 
//! Macro inactive 
	#define msg_fct( ... ) 
#endif

#define RRM_FREE(p)       if ( (p) != NULL) { free(p) ; p=NULL ; }
#define RRM_MALLOC(t,n)   (t *) malloc( sizeof(t) * n ) 
#define RRM_CALLOC(t,n)   (t *) calloc( n  , sizeof(t)) 
#define RRM_MALLOC_STR(n) RRM_MALLOC(char,n+1)

void print_L2_id(	L2_ID *id );
void print_L3_id(L3_INFO_T type,unsigned char *L3_info);
void printHex( char *msg, int len , int opt);
double get_currentclock(  ) ;


#ifdef __cplusplus
}
#endif

#endif /* RRM_UTIL_H */
