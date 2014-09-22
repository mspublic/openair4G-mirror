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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "OctetString.h"

#ifndef AUTHENTICATION_FAILURE_PARAMETER_H_
#define AUTHENTICATION_FAILURE_PARAMETER_H_

#define AUTHENTICATION_FAILURE_PARAMETER_MINIMUM_LENGTH 16
#define AUTHENTICATION_FAILURE_PARAMETER_MAXIMUM_LENGTH 16

typedef struct AuthenticationFailureParameter_tag {
    OctetString auts;
} AuthenticationFailureParameter;

int encode_authentication_failure_parameter(AuthenticationFailureParameter *authenticationfailureparameter, uint8_t iei, uint8_t *buffer, uint32_t len);

int decode_authentication_failure_parameter(AuthenticationFailureParameter *authenticationfailureparameter, uint8_t iei, uint8_t *buffer, uint32_t len);

void dump_authentication_failure_parameter_xml(AuthenticationFailureParameter *authenticationfailureparameter, uint8_t iei);

#endif /* AUTHENTICATION FAILURE PARAMETER_H_ */

