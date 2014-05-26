/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2012 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fr/openairinterface
  Address      : EURECOM, Campus SophiaTech, 450 Route des Chappes
                 06410 Biot FRANCE

*******************************************************************************/

#include <pthread.h>
#include <stdint.h>

#include "mme_default_values.h"

#ifndef MME_CONFIG_H_
#define MME_CONFIG_H_

#define MME_CONFIG_STRING_MME_CONFIG                     "MME"
#define MME_CONFIG_STRING_REALM                          "REALM"
#define MME_CONFIG_STRING_MAXENB                         "MAXENB"
#define MME_CONFIG_STRING_MAXUE                          "MAXUE"
#define MME_CONFIG_STRING_RELATIVE_CAPACITY              "RELATIVE_CAPACITY"
#define MME_CONFIG_STRING_STATISTIC_TIMER                "MME_STATISTIC_TIMER"
#define MME_CONFIG_STRING_EMERGENCY_ATTACH_SUPPORTED     "EMERGENCY_ATTACH_SUPPORTED"
#define MME_CONFIG_STRING_UNAUTHENTICATED_IMSI_SUPPORTED "UNAUTHENTICATED_IMSI_SUPPORTED"

#define MME_CONFIG_STRING_INTERTASK_INTERFACE_CONFIG     "INTERTASK_INTERFACE"
#define MME_CONFIG_STRING_INTERTASK_INTERFACE_QUEUE_SIZE "ITTI_QUEUE_SIZE"

#define MME_CONFIG_STRING_S6A_CONFIG                     "S6A"
#define MME_CONFIG_STRING_S6A_CONF_FILE_PATH             "S6A_CONF"

#define MME_CONFIG_STRING_SCTP_CONFIG                    "SCTP"
#define MME_CONFIG_STRING_SCTP_INSTREAMS                 "SCTP_INSTREAMS"
#define MME_CONFIG_STRING_SCTP_OUTSTREAMS                "SCTP_OUTSTREAMS"


#define MME_CONFIG_STRING_S1AP_CONFIG                    "S1AP"
#define MME_CONFIG_STRING_S1AP_OUTCOME_TIMER             "S1AP_OUTCOME_TIMER"

#define MME_CONFIG_STRING_GUMMEI_CONFIG                  "GUMMEI"
#define MME_CONFIG_STRING_MME_CODE                       "MME_CODE"
#define MME_CONFIG_STRING_MME_GID                        "MME_GID"
#define MME_CONFIG_STRING_PLMN                           "PLMN"
#define MME_CONFIG_STRING_MCC                            "MCC"
#define MME_CONFIG_STRING_MNC                            "MNC"
#define MME_CONFIG_STRING_TAC                            "TAC"

#define MME_CONFIG_STRING_NETWORK_INTERFACES_CONFIG      "NETWORK_INTERFACES"
#define MME_CONFIG_STRING_INTERFACE_NAME_FOR_S1_MME      "MME_INTERFACE_NAME_FOR_S1_MME"
#define MME_CONFIG_STRING_IPV4_ADDRESS_FOR_S1_MME        "MME_IPV4_ADDRESS_FOR_S1_MME"
#define MME_CONFIG_STRING_INTERFACE_NAME_FOR_S11_MME     "MME_INTERFACE_NAME_FOR_S11_MME"
#define MME_CONFIG_STRING_IPV4_ADDRESS_FOR_S11_MME       "MME_IPV4_ADDRESS_FOR_S11_MME"


typedef struct mme_config_s {
    /* Reader/writer lock for this configuration */
    pthread_rwlock_t rw_lock;

    uint8_t verbosity_level;

    char *config_file;
    char *realm;
    int   realm_length;

    uint32_t max_eNBs;
    uint32_t max_ues;

    uint8_t relative_capacity;

    uint32_t mme_statistic_timer;

    uint8_t emergency_attach_supported;
    uint8_t unauthenticated_imsi_supported;

    struct {
        uint16_t  nb_mme_gid;
        uint16_t *mme_gid;

        uint16_t  nb_mmec;
        uint8_t  *mmec;

        uint8_t   nb_plmns;
        uint16_t *plmn_mcc;
        uint16_t *plmn_mnc;
        uint16_t *plmn_mnc_len;
        uint16_t *plmn_tac;
    } gummei;

    struct {
        uint16_t in_streams;
        uint16_t out_streams;
    } sctp_config;
    struct {
        uint16_t port_number;
    } gtpv1u_config;
    struct {
        uint16_t port_number;
        uint8_t  outcome_drop_timer_sec;
    } s1ap_config;
    struct {
        uint32_t  sgw_ip_address_for_S1u_S12_S4_up;

        char     *mme_interface_name_for_S1_MME;
        uint32_t  mme_ip_address_for_S1_MME;

        char     *mme_interface_name_for_S11;
        uint32_t  mme_ip_address_for_S11;

        uint32_t  sgw_ip_address_for_S11;
    } ipv4;
    struct {
        char *conf_file;
    } s6a_config;
    struct {
        uint32_t  queue_size;
        char     *log_file;
    } itti_config;
} mme_config_t;

extern mme_config_t mme_config;

int config_parse_opt_line(int argc, char *argv[], mme_config_t *mme_config);

#define config_read_lock(mMEcONFIG)  pthread_rwlock_rdlock(&(mMEcONFIG)->rw_lock)
#define config_write_lock(mMEcONFIG) pthread_rwlock_wrlock(&(mMEcONFIG)->rw_lock)
#define config_unlock(mMEcONFIG)     pthread_rwlock_unlock(&(mMEcONFIG)->rw_lock)

//int yyparse(struct mme_config_s *mme_config_p);

#endif /* MME_CONFIG_H_ */
