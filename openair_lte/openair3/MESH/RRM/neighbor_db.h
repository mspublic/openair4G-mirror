/*!
*******************************************************************************

\file    	neighbor_db.h

\brief   	Fichier d'entete contenant les declarations des types, des defines ,
			et des fonctions relatives a la gestion de la base de donnée sur le 
			voisinage  directe (1 saut) du cluster head et indirecte a 2 sauts.

\author  	BURLOT Pascal

\date    	29/08/08

   
\par     Historique:
			$Author$  $Date$  $Revision$
			$Id$
			$Log$

*******************************************************************************
*/

#ifndef NEIGHBOR_DB_H
#define NEIGHBOR_DB_H

#ifdef __cplusplus
extern "C" {
#endif


/*!
*******************************************************************************
\brief 	Description de la structure de voisinage d'un noeud attache a un cluster
*/
typedef struct  neighbor_desc_s { 
	L2_ID 				L2_id 			; /*!< L2_id of a node               */
	// Note: suppression du champ RSSI  a ce niveau car le RSSI CH-MR n'est 
	// a priori pas mesuré , seul le RSSI des mesures de voisinage  ou le RSSI 
	// sur les canaux logiques (RB) logiques 
	//unsigned char 		RSSI			; /*!< RSSI (minus in dBm) with the CH  */
	
	unsigned int 		NB_neighbor 	; /*!< Number de neighbor (2hops) 	 */
	SENSING_MEAS_T		*Sensing_meas 	; /*!< RSSI measurement information 
											  and list of neighbor L2_id     */
	struct  neighbor_desc_s   *next 	; /*!< Next neighbor to 1 hop 		 */
} neighbor_desc_t ;

/*!
*******************************************************************************
\brief 	Description de la liste de voisinage par couple de voisin
*/
typedef struct neighbor_list_s              
{
	L2_ID 					L2_id[2] ; 		/*!< L2_id of a neighbor couple  */
	unsigned char			Rssi[2]  ; 		/*!< Rssi of each neighbor		 */
	unsigned int			nb_opened_rb; 	/*!< Number of opened RBs		 */
	rrc_rb_meas_ind_t		*rb_meas	;	/*!< Measure of opened RBs		 */
	struct neighbor_list_s 	*next ;			/*!< Next item           		 */
} neighbor_list_t ;

// ---------------------------------------------------------------------------

neighbor_desc_t *add_neighbor( neighbor_desc_t **neighbor_entry, L2_ID *L2_id ) ;
void del_neighbor( neighbor_desc_t **neighbor_entry, L2_ID *L2_id ) ;
void del_all_neighbor( neighbor_desc_t **neighbor_entry ) ;
neighbor_desc_t *get_neighbor( neighbor_desc_t *neighbor_entry, L2_ID *L2_id ) ;

void set_Sensing_meas_neighbor( neighbor_desc_t *neighbor_entry, L2_ID *L2_id , 
								unsigned int nb_meas, SENSING_MEAS_T *sensing_meas ) ;
								

unsigned char get_RSSI_neighbor(neighbor_desc_t *neighbor_entry,L2_ID *L2_id, L2_ID *neighbor_id );

// ---------------------------------------------------------------------------
/* NOT USED
neighbor_list_t *create_neighbor_list( 	neighbor_desc_t *neighbor_entry, L2_ID *L2_id_CH );
void del_neighbor_list( neighbor_list_t **neighbor_list_entry );
*/

#ifdef __cplusplus
}
#endif

#endif /* NEIGHBOR_DB_H */
