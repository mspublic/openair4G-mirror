/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/
/*! \file rlc_um_very_simple_test.h
* \brief This file defines the prototypes of the functions dealing with the sending of self generated packet for very basic test or debug of RLC or lower layers.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \note
* \bug
* \warning
*/
#    ifndef __RLC_UM_VERY_SIMPLE_TEST_H__
#        define __RLC_UM_VERY_SIMPLE_TEST_H__
//-----------------------------------------------------------------------------
#        include "rlc_um_entity.h"
#        include "mem_block.h"
#        include "rrm_config_structs.h"
#        include "rlc_um_structs.h"
#        include "rlc_um_constants.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_VERY_SIMPLE_TEST_C
#            define private_rlc_um_very_simple_test(x)    x
#            define protected_rlc_um_very_simple_test(x)  x
#            define public_rlc_um_very_simple_test(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_very_simple_test(x)
#                define protected_rlc_um_very_simple_test(x)  extern x
#                define public_rlc_um_very_simple_test(x)     extern x
#            else
#                define private_rlc_um_very_simple_test(x)
#                define protected_rlc_um_very_simple_test(x)
#                define public_rlc_um_very_simple_test(x)     extern x
#            endif
#        endif
#define RLC_UM_TEST_SDU_TYPE_TCPIP 0
#define RLC_UM_TEST_SDU_TYPE_VOIP  1
#define RLC_UM_TEST_SDU_TYPE_SMALL 2

#define tcip_sdu  "NOS TESTS MONTRENT QUE LE NOUVEAU TOSHIBA MK7559GSXP, UN DISQUE DUR DE 750 GO FONCTIONNANT AVEC DES SECTEURS DE 4 KO, EST AU MOINS AUSSI RAPIDE QUE SON PRÉDÉCESSEUR, LE MK6465GSX 640 GO, DANS TOUS LES BENCHMARKS THÉORIQUES. SES PERFORMANCES EN E/S ET SON TEMPS D’ACCÈS SONT COMPARABLES ET SON DÉBIT RÉEL EST MÊME NETTEMENT PLUS ÉLEVÉ. SES RÉSULTATS SOUS PCMARK VANTAGE, PAR CONTRE, SONT QUELQUE PEU MOINS BONS. DANS CE CAS, LEQUEL CHOISIR ? LES SCORES OBTENUS DANS LES TESTS THÉORIQUES NOUS CONFIRMENT QUE LE NOUVEAU MODÈLE SE COMPORTE CONVENABLEMENT « MALGRÉ » SES SECTEURS DE 4 KO ET QUE LA RAISON DU LÉGER RECUL DE SES PERFORMANCES SOUS PCMARK VANTAGE SE TROUVE AILLEURS. L’ALIGNEMENT DES SECTEURS N’EST PAS NON PLUS EN CAUSE, ÉTANT DONNÉ QUE WINDOWS VISTA (NOTRE OS DE TEST) ET WINDOWS 7 EN TIENNENT COMPTE LORS DE LA CRÉATION DES PARTITIONS — CE QUE NOUS AVONS BIEN ENTENDU VÉRIFIÉ INDÉPENDAMMENT.IL NOUS EST TOUTEFOIS IMPOSSIBLE DE CONTRÔLER L’EXÉCUTION ET L’ORGANISATION DE L’ÉCRITURE DES DONNÉES. PCMARK VANTAGE N’A EN EFFET JAMAIS ÉTÉ OPTIMISÉ POUR L’ÉCRITURE DE BLOCS DE DONNÉES DE GRANDE TAILLE ; DANS LA VIE RÉELLE, SI VOUS ÉCRIVEZ SURTOUT DE GROS FICHIERS, LE NOUVEAU DISQUE DUR DE 750 GO VA S’AVÉRER PLUS RAPIDE QUE LE 640 GO ET SURTOUT QUE LES RÉSULTATS ENREGISTRÉS DANS NOTRE BENCHMARK PCMARK, CAR SES SECTEURS DE 4 KO SERONT TOUJOURS PLUS PETITS QUE LES DONNÉES À ÉCRIRE. LE PROBLÈME EST QU’AUSSI LONGTEMPS QUE LES APPLICATIONS CONTINUERONT À EFFECTUER DES DEMANDES D’ÉCRITURE EN MODE  512 OCTETS"

#define voip_sdu  "Nos tests montrent que le nouveau Toshiba MK7559GSXP, un disque dur de 750 Go"
#define very_small_sdu "NoS tEsTs MoNtReNt"

public_rlc_um_very_simple_test(void rlc_um_test_send_sdu (rlc_um_entity_t* rlcP,  uint32_t frame, unsigned int sdu_typeP));
#    endif
