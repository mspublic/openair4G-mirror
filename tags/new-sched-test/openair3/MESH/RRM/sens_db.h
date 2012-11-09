/*!
*******************************************************************************

\file       sens_db.h

\brief      Fichier d'entete contenant les declarations des types, des defines ,
            et des fonctions relatives a la gestion de la base de donnée sur 
            les informations de sensing par chaque noeud avec capacité de sensing.

\author     IACOBELLI Lorenzo

\date       16/10/09

   
\par     Historique:
            $Author$  $Date$  $Revision$
            $Id$
            $Log$

*******************************************************************************
*/
#ifndef SENS_DB_H
#define SENSE_DB_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\brief Entete de la file des messages reçus ou a envoyer		
*/
typedef struct Sens_node_s { 
	L2_ID                L2_id     ; ///< id
	double               info_time ; ///< information age 
	unsigned int         Nb_chan   ; ///< number of channel informations 
	unsigned int         tpc       ; ///< time on each carrier           //mod_lor_10_02_19
	unsigned int         overlap   ; ///< overlap factor (percentage)    //mod_lor_10_02_19
	unsigned int         sampl_freq; ///< sampling frequency (Ms/s)      //mod_lor_10_02_19
	Sens_ch_t            *info_hd  ; ///< header of the information list
	struct Sens_node_s   *next     ; ///< next node pointer
} Sens_node_t ;


// ---------------------------------------------------------------------------


Sens_node_t *add_node( Sens_node_t **node_entry, L2_ID *L2_id, unsigned int NB_info, Sens_ch_t *ch_info_hd, double info_time ) ;
Sens_ch_t *add_chann( Sens_ch_t **ch_entry, unsigned int Start_f, unsigned int Final_f, unsigned int Ch_id, char *I0 ,char *mu0, char *mu1, unsigned int *is_free);        //mod_lor_10_05_28 ->char instead of int
void del_channels_info( Sens_ch_t **ch_info_entry);
void del_node( Sens_node_t **node_entry, L2_ID *L2_id ) ;
void del_all_nodes( Sens_node_t **node_entry ) ;
Sens_node_t *get_node_info ( Sens_node_t *node_entry, L2_ID *L2_id ) ;
Sens_ch_t *get_chann_info( Sens_ch_t *ch_entry , unsigned int Ch_id); //mod_lor_10_03_17: intxflot
Sens_node_t *update_node_info( Sens_node_t **node_entry, L2_ID *L2_id, unsigned int NB_info, Sens_ch_t *ch_info_hd, double info_time);
Sens_node_t *update_node_par( Sens_node_t **node_entry, L2_ID *L2_id,  unsigned int NB_info, Sens_ch_t *ch_info_hd, double info_time,
                    unsigned int tpc, unsigned int overlap, unsigned int sampl_freq); //mod_lor_10_02_19 : function creation
//Sens_ch_t *update_channel_info( Sens_ch_t **ch_entry, unsigned int Start_f, unsigned int Final_f, unsigned int Ch_id, float meas, unsigned int is_free );//mod_lor_10_05_28 ->char instead of int
Sens_ch_t *update_channel_info( Sens_ch_t **ch_entry, unsigned int Start_f, unsigned int Final_f, unsigned int Ch_id, char *I0 ,char *mu0, char *mu1, unsigned int *is_free ); //mod_lor_10_05_28 ->char instead of int
void print_sens_db( Sens_node_t *pEntry );


#ifdef __cplusplus
}
#endif

#endif /* SENS_DB_H */
