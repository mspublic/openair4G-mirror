/*
 * util.hpp
 *
 *  Created on: May 2, 2012
 *      Author: demiray
 */

#ifndef MGMT_UTIL_HPP_
#define MGMT_UTIL_HPP_

#include <sys/types.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>
using namespace std;

/**
 * A container for utility methods for bit/byte processing and formatted printing
 */
class Util {

	public:
		/**
		 * Fills incoming buffer with 0x00es
		 *
		 * @param buffer Buffer to be reset
		 * @param bufferSize Size of the buffer
		 * @return none
		 */
		static void resetBuffer(void* buffer, size_t bufferSize);
		/**
		 * Copies data between given two buffers
		 *
		 * @param destinationBuffer Buffer that data will be copied into
		 * @param sourceBuffer Buffer that data will be copied from
		 * @param copySize Amount of data to be copied
		 * @return true on success, false otherwise
		 */
		static bool copyBuffer(void* destinationBuffer, const void* sourceBuffer, size_t copySize);
		/**
		 * Prints hexadecimal representation of given buffer
		 *
		 * @param buffer Buffer that will be printed out
		 * @param bufferSize Size of the buffer
		 * @return true on success, false otherwise
		 */
		static bool printHexRepresentation(unsigned char* buffer, unsigned long bufferSize);
		/**
		 * Prints binary representation of given octet
		 *
		 * @param message Text message that'll be written before writing octet's content
		 * @param octet Octet to be printed out
		 * @return none
		 */
		static void printBinaryRepresentation(unsigned char* message, u_int8_t octet);
		/**
		 * Sets Nth bit of given octet
		 *
		 * @param octet Pointer to the octet
		 * @param index Index that'll be set
		 * @return true on success, false otherwise
		 */
		static bool setBit(u_int8_t& octet, u_int8_t index);
		/**
		 * Checks if Nth bit of given octet is set
		 *
		 * @param octet Octet
		 * @param index Index that'll be checked
		 * @return true on success, false otherwise
		 */
		static bool isBitSet(u_int8_t octet, u_int8_t index);
		/**
		 * Parses 8-byte integer data from given buffer
		 *
		 * @param buffer Buffer that 8-byte integer will be parsed from
		 * @param integer Integer buffer that parsed data will be copied
		 * @return true on success, false otherwise
		 */
		static bool parse8byteInteger(const unsigned char* buffer, u_int64_t* integer);
		/**
		 * Parses 4-byte integer data from given buffer
		 *
		 * @param buffer Buffer that 4-byte integer will be parsed from
		 * @param integer Integer buffer that parsed data will be copied
		 * @return true on success, false otherwise
		 */
		static bool parse4byteInteger(const unsigned char* buffer, u_int32_t* integer);
		/**
		 * Parses 2-byte integer data from given buffer
		 *
		 * @param buffer Buffer that 2-byte integer will be parsed from
		 * @param integer Integer buffer that parsed data will be copied
		 * @return true on success, false otherwise
		 */
		static bool parse2byteInteger(const unsigned char* buffer, u_int16_t* integer);
		/**
		 * Encodes given 8-byte integer data into buffer at given index
		 *
		 * @param buffer Vector that 8-byte integer will be encoded into
		 * @param bufferIndex Index that 8-byte integer will be encoded at
		 * @param data 8-byte integer data that will be encoded
		 * @return true on success, false otherwise
		 */
		static bool encode8byteInteger(vector<unsigned char>& buffer, u_int16_t bufferIndex, u_int64_t data);
		/**
		 * Encodes given 4-byte integer data into buffer at given index
		 *
		 * @param buffer Vector that 4-byte integer will be encoded into
		 * @param bufferIndex Index that 4-byte integer will be encoded at
		 * @param data 4-byte integer data that will be encoded
		 * @return true on success, false otherwise
		 */
		static bool encode4byteInteger(vector<unsigned char>& buffer, u_int16_t bufferIndex, u_int32_t data);
		/**
		 * Encodes given 2-byte integer data into buffer at given index
		 *
		 * @param buffer Vector that 2-byte integer will be encoded into
		 * @param bufferIndex Index that 2-byte integer will be encoded at
		 * @param data 2-byte integer data that will be encoded
		 * @return true on success, false otherwise
		 */
		static bool encode2byteInteger(vector<unsigned char>& buffer, u_int16_t bufferIndex, u_int16_t data);
		/**
		 * Splits given string with given delimiter
		 *
		 * @param input Input string that is going to be split
		 * @param delimiter Delimiter character
		 * @return std::vector containing split parts
		 */
		static vector<string> split(const string& input, char delimiter);
};

#endif /* MGMT_UTIL_HPP_ */
