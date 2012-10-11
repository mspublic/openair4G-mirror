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
  Address      : EURECOM, Campus SophiaTech, 450 Route des Chappes, 06410 Biot FRANCE

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
#include "packets/mgmt_gn_packet_location_update.hpp"
#include "mgmt_inquiry_thread.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include "util/mgmt_util.hpp"
#include <boost/thread.hpp>
#include <iostream>
using namespace std;

InquiryThread::InquiryThread(ManagementInformationBase& mib, UdpServer& connection, u_int8_t wirelessStateUpdateInterval, u_int8_t locationUpdateInterval, Logger& logger)
	: connection(connection), mib(mib), logger(logger) {
	this->wirelessStateUpdateInterval = wirelessStateUpdateInterval;
	this->locationUpdateInterval = locationUpdateInterval;
}

InquiryThread::~InquiryThread() {
}

void InquiryThread::operator()() {
	/**
	 * Find smaller interval and the difference between them
	 */
	if (wirelessStateUpdateInterval == locationUpdateInterval) {
		boost::posix_time::seconds wait(wirelessStateUpdateInterval);

		while (true) {
			logger.info("Waiting for " + boost::lexical_cast<string>((int)wirelessStateUpdateInterval) + " second(s) to send a Wireless State Update and a Location Update");
			boost::this_thread::sleep(wait);
			if (!requestWirelessStateUpdate() || !requestLocationUpdate())
				break;
		}

	} else if (wirelessStateUpdateInterval > locationUpdateInterval) {
		boost::posix_time::seconds wait(locationUpdateInterval), difference(wirelessStateUpdateInterval - locationUpdateInterval);

		while (true) {
			logger.info("Waiting for " + boost::lexical_cast<string>((int)locationUpdateInterval) + " second(s) to send a Location Update");
			boost::this_thread::sleep(wait);
			if (!requestLocationUpdate())
				break;
			logger.info("Waiting for " + boost::lexical_cast<string>((int)wirelessStateUpdateInterval - locationUpdateInterval) + " second(s) to send a Wireless Update");
			boost::this_thread::sleep(difference);
			if (!requestWirelessStateUpdate())
				break;
		}
	} else {
		boost::posix_time::seconds wait(wirelessStateUpdateInterval), difference(locationUpdateInterval - wirelessStateUpdateInterval);

		while (true) {
			logger.info("Waiting for " + boost::lexical_cast<string>((int)wirelessStateUpdateInterval) + " second(s) to send a Wireless State Update");
			boost::this_thread::sleep(wait);
			if (!requestWirelessStateUpdate())
				break;

			logger.info("Waiting for " + boost::lexical_cast<string>((int)locationUpdateInterval - wirelessStateUpdateInterval) + " second(s) to send a Location Update");
			boost::this_thread::sleep(difference);
			if (!requestLocationUpdate())
				break;
		}
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

bool InquiryThread::requestLocationUpdate() {
	GeonetLocationUpdateEventPacket request(mib, logger);

	if (connection.send(request)) {
		logger.info("Location Update message has been sent");

		return true;
	} else {
		logger.error("Location Update message cannot be sent!");

		return false;
	}
}
