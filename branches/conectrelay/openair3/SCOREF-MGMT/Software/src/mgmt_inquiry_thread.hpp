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
 * \file mgmt_inquiry_thread.hpp
 * \brief A thread worker function to ask repetitive questions to relevant modules to update MIB
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#ifndef MGMT_INQUIRY_THREAD_HPP_
#define MGMT_INQUIRY_THREAD_HPP_

#include "util/mgmt_udp_server.hpp"
#include "util/mgmt_log.hpp"

class InquiryThread {
	public:
		/**
		 * Constructor for InquiryThread class
		 *
		 * @param mib Management Information Base reference
		 * @param connection UdpServer object that the questions will be asked through
		 * @param wirelessStateUpdateInterval Wireless State Update interval in seconds
		 * @param locationUpdateInterval Location Update interval in seconds
		 * @param logger Logger object reference
		 */
		InquiryThread(ManagementInformationBase& mib, UdpServer& connection, u_int8_t wirelessStateUpdateInterval, u_int8_t locationUpdateInterval, Logger& logger);
		/**
		 * Destructor for InquiryThread class
		 */
		virtual ~InquiryThread();

	public:
		/**
		 * () operator overload to pass this method to boost::thread
		 *
		 * todo this method is too complex and prone to errors, should be revised
		 */
		void operator()();
		/**
		 * Sends request for a Wireless State Response message
		 * Incoming message will be handled and MIB will be updated
		 * accordingly by GeonetMessageHandler class
		 */
		bool requestWirelessStateUpdate();
		/**
		 * Sends a request for Location Update
		 * Incoming message will be handled and MIB will be updated
		 * accordingly by GeonetMessageHandler class
		 */
		bool requestLocationUpdate();

	private:
		/**
		 * UdpServer object reference to communicate with client
		 */
		UdpServer& connection;
		/**
		 * Wireless State Update interval in seconds
		 */
		u_int8_t wirelessStateUpdateInterval;
		/**
		 * Location Update interval in seconds
		 */
		u_int8_t locationUpdateInterval;
		/**
		 * Management Information Base reference
		 */
		ManagementInformationBase& mib;
		/**
		 * Logger object reference
		 */
		Logger& logger;
};


#endif /* MGMT_INQUIRY_THREAD_HPP_ */