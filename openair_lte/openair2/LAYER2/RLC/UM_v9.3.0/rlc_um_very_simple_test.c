#define RLC_UM_MODULE
#define RLC_UM_VERY_SIMPLE_TEST_C
#include <string.h>
#include "rtos_header.h"
#include "platform_types.h"

#include "list.h"
#include "rlc_um.h"
#include "rlc.h"
#include "rlc_primitives.h"
#include "mac_primitives.h"
#include "LAYER2/MAC/extern.h"


char tcip_sdu[] = "NOS TESTS MONTRENT QUE LE NOUVEAU TOSHIBA MK7559GSXP, UN DISQUE DUR DE 750 GO FONCTIONNANT AVEC DES SECTEURS DE 4 KO, EST AU MOINS AUSSI RAPIDE QUE SON PRÉDÉCESSEUR, LE MK6465GSX 640 GO, DANS TOUS LES BENCHMARKS THÉORIQUES. SES PERFORMANCES EN E/S ET SON TEMPS D’ACCÈS SONT COMPARABLES ET SON DÉBIT RÉEL EST MÊME NETTEMENT PLUS ÉLEVÉ. SES RÉSULTATS SOUS PCMARK VANTAGE, PAR CONTRE, SONT QUELQUE PEU MOINS BONS. DANS CE CAS, LEQUEL CHOISIR ? LES SCORES OBTENUS DANS LES TESTS THÉORIQUES NOUS CONFIRMENT QUE LE NOUVEAU MODÈLE SE COMPORTE CONVENABLEMENT « MALGRÉ » SES SECTEURS DE 4 KO ET QUE LA RAISON DU LÉGER RECUL DE SES PERFORMANCES SOUS PCMARK VANTAGE SE TROUVE AILLEURS. L’ALIGNEMENT DES SECTEURS N’EST PAS NON PLUS EN CAUSE, ÉTANT DONNÉ QUE WINDOWS VISTA (NOTRE OS DE TEST) ET WINDOWS 7 EN TIENNENT COMPTE LORS DE LA CRÉATION DES PARTITIONS — CE QUE NOUS AVONS BIEN ENTENDU VÉRIFIÉ INDÉPENDAMMENT.IL NOUS EST TOUTEFOIS IMPOSSIBLE DE CONTRÔLER L’EXÉCUTION ET L’ORGANISATION DE L’ÉCRITURE DES DONNÉES. PCMARK VANTAGE N’A EN EFFET JAMAIS ÉTÉ OPTIMISÉ POUR L’ÉCRITURE DE BLOCS DE DONNÉES DE GRANDE TAILLE ; DANS LA VIE RÉELLE, SI VOUS ÉCRIVEZ SURTOUT DE GROS FICHIERS, LE NOUVEAU DISQUE DUR DE 750 GO VA S’AVÉRER PLUS RAPIDE QUE LE 640 GO ET SURTOUT QUE LES RÉSULTATS ENREGISTRÉS DANS NOTRE BENCHMARK PCMARK, CAR SES SECTEURS DE 4 KO SERONT TOUJOURS PLUS PETITS QUE LES DONNÉES À ÉCRIRE. LE PROBLÈME EST QU’AUSSI LONGTEMPS QUE LES APPLICATIONS CONTINUERONT À EFFECTUER DES DEMANDES D’ÉCRITURE EN MODE  512 OCTETS";

char voip_sdu[] = "Nos tests montrent que le nouveau Toshiba MK7559GSXP, un disque dur de 750 Go";
char very_small_sdu[] = "NoS tEsTs MoNtReNt";

//-----------------------------------------------------------------------------
void
rlc_um_test_send_sdu     (rlc_um_entity_t* rlcP,  unsigned int sdu_typeP) {
//-----------------------------------------------------------------------------
    mem_block_t *sdu_mem;
    switch (sdu_typeP) {
        case RLC_UM_TEST_SDU_TYPE_TCPIP:
            sdu_mem = get_free_mem_block (strlen(tcip_sdu)+ 1 + sizeof (struct rlc_um_data_req_alloc));
            if (sdu_mem != NULL) {
                memset (sdu_mem->data, 0, sizeof (struct rlc_um_data_req_alloc));
                ((struct rlc_um_data_req*)(sdu_mem->data))->data_size      = strlen(tcip_sdu)+ 1;
                ((struct rlc_um_data_req*)(sdu_mem->data))->data_offset    = 0;
                ((struct rlc_um_data_req*)(sdu_mem->data))->use_special_li = sizeof (struct rlc_um_data_req_alloc) - sizeof (struct rlc_um_data_req);
                strcpy (&sdu_mem->data[sizeof (struct rlc_um_data_req_alloc)], tcip_sdu);
                rlc_um_data_req     (rlcP,  sdu_mem);
            }
            break;
        case RLC_UM_TEST_SDU_TYPE_VOIP:
            sdu_mem = get_free_mem_block (strlen(voip_sdu)+ 1 + sizeof (struct rlc_um_data_req_alloc));
            if (sdu_mem != NULL) {
                memset (sdu_mem->data, 0, sizeof (struct rlc_um_data_req_alloc));
                ((struct rlc_um_data_req*)(sdu_mem->data))->data_size      = strlen(voip_sdu)+ 1;
                ((struct rlc_um_data_req*)(sdu_mem->data))->data_offset    = sizeof (struct rlc_um_data_req_alloc) - sizeof (struct rlc_um_data_req);
                ((struct rlc_um_data_req*)(sdu_mem->data))->use_special_li = 0;
                strcpy (&sdu_mem->data[sizeof (struct rlc_um_data_req_alloc)], voip_sdu);
                rlc_um_data_req     (rlcP,  sdu_mem);
            }
            break;
        case RLC_UM_TEST_SDU_TYPE_SMALL:
            sdu_mem = get_free_mem_block (strlen(very_small_sdu)+ 1 + sizeof (struct rlc_um_data_req_alloc));
            if (sdu_mem != NULL) {
                memset (sdu_mem->data, 0, sizeof (struct rlc_um_data_req_alloc));
                ((struct rlc_um_data_req*)(sdu_mem->data))->data_size      = strlen(very_small_sdu)+ 1;
                ((struct rlc_um_data_req*)(sdu_mem->data))->data_offset    = sizeof (struct rlc_um_data_req_alloc) - sizeof (struct rlc_um_data_req);
                ((struct rlc_um_data_req*)(sdu_mem->data))->use_special_li = 0;
                strcpy (&sdu_mem->data[sizeof (struct rlc_um_data_req_alloc)], very_small_sdu);
                rlc_um_data_req     (rlcP,  sdu_mem);
            }
            break;
        default:;
    }
}
