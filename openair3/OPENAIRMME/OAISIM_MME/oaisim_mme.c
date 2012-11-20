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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "mme_config.h"

#include "intertask_interface.h"
#include "sctp_primitives_server.h"
#include "udp_primitives_server.h"
#include "s1ap_mme.h"
#include "gtpv1_u.h"
#include "log.h"
#include "timer.h"

int main(int argc, char *argv[])
{
    mme_config_t mme_config;

    config_parse_opt_line(argc, argv, &mme_config);
    fprintf(stdout, "Starting %s\n", PACKAGE_STRING);

    /* Calling each layer init function */
    log_init(&mme_config);
    intertask_interface_init(&mme_config);
    sctp_init(&mme_config);
    udp_init(&mme_config);
    s1ap_mme_init(&mme_config);
    gtpv1_u_init(&mme_config);
    timer_init(&mme_config);

    while(1) {
        sleep(1);
    }

    return 0;
}
