/*
 * mgmt_client.hpp
 *
 *  Created on: 14 Jun 2012
 *      Author: barisd
 */

#ifndef MGMT_CLIENT_HPP_
#define MGMT_CLIENT_HPP_

#include <boost/asio.hpp>
using boost::asio::ip::udp;

/**
 * A container to hold information about Management clients, mostly used
 * in Message Handler code
 */
class ManagementClient {
	public:
		/**
		 * Client state
		 */
		enum ManagementClientState {
			/**
			 * No client has been connected
			 */
			OFFLINE = 0,
			/**
			 * A client has been connected but has not yet received configuration
			 */
			ONLINE = 1,
			/**
			 * A client has been connected and has received configuration
			 */
			CONNECTED = 2
		};

	public:
		/**
		 * Constructor for ManagementClient class
		 *
		 * @param client Socket information of relevant client
		 */
		ManagementClient(const udp::endpoint& client) {
			this->client = client;
		}
		/**
		 * Destructor for ManagementClient class
		 */
		~ManagementClient() {}

	public:
		/**
		 * Getter for IP address of this client
		 *
		 * @return IP address of this client
		 */
		boost::asio::ip::address getAddress() const {
			return this->client.address();
		}
		/**
		 * Getter for port number of this client
		 *
		 * @return Port number of this client
		 */
		unsigned short int getPort() const {
			return this->client.port();
		}

		/**
		 * Overloaded == operator to use ManagementClient type as a std::map key
		 *
		 * @param client Client that is going to be compared with
		 * @return true if clients are the same, false otherwise
		 */
		bool operator==(const ManagementClient& client) const {
			if (this->client.address() == client.getAddress())
				return true;

			return false;
		}
		/**
		 * Overloaded < operator to use ManagementClient type as a std::map key
		 *
		 * @param client Client that is going to be compared with
		 * @return true if host object's IP address is smaller, false otherwise
		 */
		bool operator<(const ManagementClient& client) const {
			if (this->client.address() < client.getAddress())
				return true;

			return false;
		}

	private:
		/**
		 * Client's UDP socket information
		 */
		udp::endpoint client;
};

#endif /* MGMT_CLIENT_HPP_ */
