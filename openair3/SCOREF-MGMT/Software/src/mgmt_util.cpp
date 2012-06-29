/*
 * mgmt_util.cpp
 *
 *  Created on: May 7, 2012
 *      Author: demiray
 */

#include "mgmt_util.hpp"

#include <sstream>
using namespace std;

void Util::resetBuffer(void* buffer, size_t bufferSize) {
	memset(buffer, 0x00, bufferSize);
}

bool Util::copyBuffer(void* destinationBuffer, const void* sourceBuffer, size_t copySize) {
	if (!destinationBuffer || !sourceBuffer)
		return false;

	memcpy(destinationBuffer, sourceBuffer, copySize);

	return true;
}

bool Util::printHexRepresentation(unsigned char* buffer, unsigned long bufferSize) {
	if (!buffer)
		return false;

	unsigned long octet_index = 0;

	cout << "     |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f |" << endl;
	cout << "-----+-------------------------------------------------|" << endl;
	cout << " 000 |";
	for (octet_index = 0; octet_index < bufferSize; ++octet_index) {
		/*
		 * Print every single octet in hexadecimal form
		 */
		cout << " " << setfill('0') << setw(2) << hex << (int) buffer[octet_index];
		/*
		 * Align newline and pipes according to the octets in groups of 2
		 */
		if (octet_index != 0 && (octet_index + 1) % 16 == 0) {
			cout << " |" << endl;
			cout << " " << setfill('0') << setw(3) << octet_index << " |";
		}
	}

	/*
	 * Append enough spaces and put final pipe
	 */
	unsigned char index;
	for (index = octet_index; index < 16; ++index)
		cout << "   ";
	cout << " |" << endl;

	cout << resetiosflags(ios_base::hex) << endl;

	return true;
}

void Util::printBinaryRepresentation(unsigned char* message, u_int8_t octet) {
	unsigned char index = 0;
	unsigned char mask = 0x80;

	cout << message;

	for (index = 0; index < 8; ++index) {
		if (octet & mask) {
			cout << "1";
		} else {
			cout << "0";
		}

		mask /= 2;
	}
	cout << endl;
}

bool Util::setBit(u_int8_t& octet, u_int8_t index) {
	u_int8_t mask = 0x80;

	/*
	 * Set relevant bit
	 */
	octet |= (mask >>= index);

	return true;
}

bool Util::isBitSet(u_int8_t octet, u_int8_t index) {
	u_int8_t mask = 0x80;

	/*
	 * Check relevant bit
	 */
	return octet &= (mask >>= index);
}

bool Util::parse8byteInteger(const unsigned char* buffer, u_int64_t* integer) {
	if (!buffer || !integer)
		return false;

	*integer = 0x00;
	*integer |= buffer[0] & 0xff; *integer <<= 8;
	*integer |= buffer[1] & 0xff; *integer <<= 8;
	*integer |= buffer[2] & 0xff; *integer <<= 8;
	*integer |= buffer[3] & 0xff; *integer <<= 8;
	*integer |= buffer[4] & 0xff; *integer <<= 8;
	*integer |= buffer[5] & 0xff; *integer <<= 8;
	*integer |= buffer[6] & 0xff; *integer <<= 8;
	*integer |= buffer[7] & 0xff;

	return true;
}

bool Util::parse4byteInteger(const unsigned char* buffer, u_int32_t* integer) {
	if (!buffer || !integer)
		return false;

	*integer = (buffer[0] << 24);
	*integer |= (buffer[1] << 16);
	*integer |= (buffer[2] << 8);
	*integer |= buffer[3] & 0xff;

	return true;
}

bool Util::parse2byteInteger(const unsigned char* buffer, u_int16_t* integer) {
	if (!buffer || !integer)
		return false;

	*integer = (buffer[0] << 8);
	*integer |= (buffer[1] & 0xff);

	return true;
}

bool Util::encode8byteInteger(vector<unsigned char>& buffer, u_int16_t bufferIndex, u_int64_t data) {
	if (buffer.size() < (unsigned)(bufferIndex - 1))
		return false;

	u_int32_t dataHigherPart = ((data >> 32) & 0xffffffff);
	u_int32_t dataLowerPart = (data & 0xffffffff);

	return encode4byteInteger(buffer, bufferIndex, dataHigherPart) && \
		encode4byteInteger(buffer, bufferIndex + sizeof(u_int32_t), dataLowerPart);
}

bool Util::encode4byteInteger(vector<unsigned char>& buffer, u_int16_t bufferIndex, u_int32_t data) {
	if (buffer.size() < (unsigned)(bufferIndex - 1))
		return false;

	buffer[bufferIndex] = ((data >> 24) & 0xff);
	buffer[bufferIndex + 1] = ((data >> 16) & 0xff);
	buffer[bufferIndex + 2] = ((data >> 8) & 0xff);
	buffer[bufferIndex + 3] = (data & 0xff);

	return true;
}

bool Util::encode2byteInteger(vector<unsigned char>& buffer, u_int16_t bufferIndex, u_int16_t data) {
	if (buffer.size() < (unsigned)(bufferIndex - 1))
		return false;

	buffer[bufferIndex] = ((data >> 8) & 0xff);
	buffer[bufferIndex + 1] = (data & 0xff);

	return true;
}

vector<string> Util::split(const string& input, char delimiter) {
	vector<string> elements;
	stringstream inputStream(input);
	string item;

	while (std::getline(inputStream, item, delimiter))
		elements.push_back(item);

	return elements;
}
