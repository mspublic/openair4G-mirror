/*!
*******************************************************************************

\file       channels_db.h

\brief      Fichier d'entete contenant les declarations des types, des defines ,
            et des fonctions relatives a la gestion de la base de donnée sur 
            les canaux.

\author     IACOBELLI Lorenzo

\date       16/10/09

   
\par     Historique:
            $Author$  $Date$  $Revision$
            $Id$
            $Log$

*******************************************************************************
*/
#ifndef CHANNELS_DB_H
#define CHANNELS_DB_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\brief Entete de la file des messages reçus ou a envoyer		
*/
typedef struct channels_db_s { 
	double               info_time ; ///< information age 
	unsigned int         is_free   ; ///< channel availability  
	unsigned int         priority  ; ///< channel priority
	unsigned int         is_ass    ; ///< channel used by secondary network
	L2_ID                source_id ; ///< SU using channel (source)
    L2_ID                dest_id   ; ///< SU using channel (dest)
	CHANNEL_T            channel   ; ///< channel description
	struct channels_db_s *next     ; ///< next node pointer
} CHANNELS_DB_T ;


// ---------------------------------------------------------------------------

CHANNELS_DB_T *add_chann_db( CHANNELS_DB_T **ch_entry, CHANNEL_T channel, unsigned int is_free, double info_time);        
void del_channel( CHANNELS_DB_T **ch_entry, unsigned int Ch_id);
void del_all_channels( CHANNELS_DB_T **ch_entry ) ;
CHANNELS_DB_T *get_chann_db_info( CHANNELS_DB_T *ch_entry  , unsigned int Ch_id );
CHANNELS_DB_T *up_chann_db( CHANNELS_DB_T **ch_entry, CHANNEL_T channel, unsigned int is_free, double info_time); 
CHANNELS_DB_T *up_chann_ass( CHANNELS_DB_T *ch_entry, unsigned int Ch_id, unsigned int is_ass, L2_ID source_id, L2_ID dest_id); 

#ifdef __cplusplus
}
#endif

#endif /* CHANNELS_DB_H */
