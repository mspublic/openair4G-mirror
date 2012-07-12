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
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*!
 * \file mgmt_inquiry_thread.cpp
 * \brief A thread worker function to ask repetitive questions to relevant modules to update MIB
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "packets/mgmt_gn_packet_wireless_state_request.hpp"
#include "mgmt_inquiry_thread.hpp"
#include <boost/date_time.hpp>
#include "util/mgmt_util.hpp"
#include <boost/thread.hpp>
#include <iostream>
using namespace std;

InquiryThread::InquiryThread(UdpServer& connection, u_int8_t wirelessStateUpdateInterval, Logger& logger)
	: connection(connection), logger(logger) {
	this->wirelessStateUpdateInterval = wirelessStateUpdateInterval;
}

InquiryThread::~InquiryThread() {
}

void InquiryThread::operator()() {
	boost::posix_time::seconds wait(wirelessStateUpdateInterval);

	while (true) {
		// todo logger.info("Waiting for " + wirelessStateUpdateInterval + " second(s)...");
		boost::this_thread::sleep(wait);

		if (!requestWirelessStateUpdate())
			break;
	}
}

bool InquiryThread::requestWirelessStateUpdate() {
	GeonetWirelessStateRequestEventPacket request(logger);

	if (connection.send(request)) {
		logger.info("Wireless state request message has been sent");

		return true;
	} else {
		logger.error("Wireless state request message cannot be sent!");

		return false;
	}
}
