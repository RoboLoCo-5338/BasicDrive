/*
 * MQServer.cpp
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#include <mqserver/MQServer.h>

namespace mqserver {

std::string MQServer::GetString(std::string name) {
	return stringMap[name];
}

double MQServer::GetDouble(std::string name) {
	return doubleMap[name];
}

long MQServer::GetLong(std::string name) {
	return longMap[name];
}

void MQServer::Set(std::string name, std::string value) {
	stringMap[name] = value; // TODO: THIS IS NOT THREAD-SAFE
	// REWRITE AS MESSAGE SENT TO MAIN MQ LOOP
}

void MQServer::Set(std::string name, double value) {
	doubleMap[name] = value;	// TODO: THIS IS NOT THREAD-SAFE
}

void MQServer::Set(std::string name, long value) {
	longMap[name] = value;	// TODO: THIS IS NOT THREAD-SAFE
}

// Format for mqserver messages
// First byte - message type (write or read)
// 0 - read
// 1 - write
// 2 bytes - name length
// name length bytes - name
// 1 byte - data type
// x bytes - data
//
// Data types
// Null   - 0
// Long   - 1
// Double - 2
// String - 3
// Long is 8 bytes
// Double is 8 bytes
// String is followed immediately by a 2-byte length, then the string

MQServer::MQServer() :
		context(new zmq::context_t(1)), sock(
				new zmq::socket_t(*context, zmq::socket_type::rep)), serverThread(
				&MQServer::Run, this) {
	// TODO Auto-generated constructor stub
}

// Get message specifies name and type
// Get response returns status and value
// Set message specifies name and type and value
// Set response returns status

// Header of an input packet
struct packetHeader {
	struct {
		uint8_t type;
		uint16_t namelen;
	} readHeader;
	uint8_t dataType;
	uint16_t datalen;
};

void MQServer::Run() {
	sock->bind("tcp://*:5810");
	while (running) {
		zmq::message_t request;

		sock->recv(&request);
		if (request.size() > sizeof(packetHeader)) {
			packetHeader *p = (mqserver::packetHeader*) request.data();
			uint8_t *bytes = (uint8_t*) request.data();
			auto namelen = p->readHeader.namelen;
			switch (p->readHeader.type) {
			case 0: { // Read
				std::string name((char*) &bytes[sizeof(p->readHeader)], namelen); // Name string starts after read header
				switch (p->readHeader.type) {
				case 0: // Null
					break;
				case 1: // Long
					GetLong(name);
					break;
				case 2: // Double
					GetDouble(name);
					break;
				case 3: // String
					GetString(name);
					break;
				default:
					break;
				}
				break;
			}
			case 1: {// Write
				break;
			}
			default: {// Unknown packet type
				break;
			}
			}
		}

	}
	sock->close();
	context->close();
}

} /* namespace mqserver */
