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

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

*******************************************************************************/
/*! \file rrc_eNB_GTPV1U.h
 * \brief rrc GTPV1U procedures for eNB
 * \author Lionel GAUTHIER
 * \version 1.0
 * \company Eurecom
 * \email: lionel.gauthier@eurecom.fr
 */

#ifndef RRC_ENB_GTPV1U_H_
#define RRC_ENB_GTPV1U_H_

# if defined(ENABLE_USE_MME)


#   if defined(ENABLE_ITTI)

/*! \fn rrc_eNB_process_GTPV1U_CREATE_TUNNEL_RESP(MessageDef *msg_pP, const char *msg_name_pP, instance_t instanceP)
 *\brief Process GTPV1U_ENB_CREATE_TUNNEL_RESP message received from GTPV1U, retrieve the enb teid created.
 *\param msg_p Message received by RRC.
 *\param msg_name Message name.
 *\param instance Message instance.
 *\return 0 when successful, -1 if the UE index can not be retrieved. */
int rrc_eNB_process_GTPV1U_CREATE_TUNNEL_RESP(MessageDef *msg_pP, const char *msg_name_pP, instance_t instanceP);

#   endif
# endif /* defined(ENABLE_USE_MME) */
#endif /* RRC_ENB_GTPV1U_H_ */
