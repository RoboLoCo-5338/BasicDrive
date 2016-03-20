/*
 * MQServer.cpp
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#include <mqserver/MQServer.h>

namespace mqserver {

// Format for mqserver messages
// First byte - message type (write or read)
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
		context(new zmq::context_t(1)),
		sock(new zmq::socket_t(*context, zmq::socket_type::rep)),
		serverThread(&MQServer::Run, this),
		map()
		{
	// TODO Auto-generated constructor stub
}

void MQServer::Run() {
	sock->bind("tcp://*:5810");
	while (running) {
		zmq::message_t request;

		sock->recv(&request);

	}
}

void MQServer::Set(std::string s, MQObject& o) {

}

MQObject* MQServer::Get(std::string s) {

}
} /* namespace mqserver */
