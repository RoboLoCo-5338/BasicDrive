/*
 * MQServer.cpp
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#include <mqserver/MQServer.h>

namespace mqserver {

struct packetHeader {
	struct {
		uint8_t type;
		uint16_t namelen;
	} readHeader;
	uint8_t dataType;
	uint16_t datalen;
};

struct getResponseHeader {
	uint8_t status;
	uint8_t dataType;
	uint16_t datalen;
};

struct setResponseHeader {
	uint8_t status;
};

std::string MQServer::GetString(std::string name) {
	return stringMap[name];
}

double MQServer::GetDouble(std::string name) {
	return doubleMap[name];
}

int64_t MQServer::GetLong(std::string name) {
	return longMap[name];
}

void MQServer::Set(std::string name, std::string value) {
	stringMap[name] = value; // TODO: THIS IS NOT THREAD-SAFE
	// REWRITE AS MESSAGE SENT TO MAIN MQ LOOP
}

void MQServer::Set(std::string name, double value) {
	doubleMap[name] = value;	// TODO: THIS IS NOT THREAD-SAFE
}

void MQServer::Set(std::string name, int64_t value) {
	longMap[name] = value;	// TODO: THIS IS NOT THREAD-SAFE
}

void MQServer::PutString(std::string name, std::string value) {
	zmq::socket_t a(*context,zmq::socket_type::req);
	a.connect("inproc://mqserver");
	packetHeader p;
	p.dataType = 3; // string
	p.datalen = value.size();
	p.readHeader.type = 1; // put/write
	p.readHeader.namelen = name.size();
	zmq::message_t req(sizeof(packetHeader) + p.datalen + p.readHeader.namelen);
	uint8_t* bytes = (uint8_t*)req.data();
	memcpy(req.data(),&p,sizeof(p));
	memcpy(&bytes[sizeof(p)],name.data(),p.readHeader.namelen);
	memcpy(&bytes[sizeof(p)+p.readHeader.namelen],value.data(),p.datalen);
	a.send(req);
	a.disconnect("inproc://mqserver");
	a.close();
}

void MQServer::PutDouble(std::string name, double value) {
	zmq::socket_t a(*context,zmq::socket_type::req);
	a.connect("inproc://mqserver");
	packetHeader p;
	p.dataType = 2; // double
	p.datalen = 8;
	p.readHeader.type = 1; // put/write
	p.readHeader.namelen = name.size();
	zmq::message_t req(sizeof(packetHeader) + p.datalen + p.readHeader.namelen);
	uint8_t* bytes = (uint8_t*)req.data();
	memcpy(req.data(),&p,sizeof(p));
	memcpy(&bytes[sizeof(p)],name.data(),p.readHeader.namelen);
	memcpy(&bytes[sizeof(p)+p.readHeader.namelen],&value,p.datalen);
	printf("Name %s = %f", name.c_str(), value);
	printf("Total Length: %d, actual %d", (sizeof(packetHeader) + p.datalen + p.readHeader.namelen), req.size());
	a.send(req);
	a.disconnect("inproc://mqserver");
	a.close();
}

void MQServer::PutLong(std::string name, int64_t value) {
	zmq::socket_t a(*context,zmq::socket_type::req);
	a.connect("inproc://mqserver");
	packetHeader p;
	p.dataType = 1; // long
	p.datalen = 8;
	p.readHeader.type = 1; // put/write
	p.readHeader.namelen = name.size();
	zmq::message_t req(sizeof(packetHeader) + p.datalen + p.readHeader.namelen);
	uint8_t* bytes = (uint8_t*)req.data();
	printf("1\n");
	memcpy(req.data(),&p,sizeof(p));
	printf("2\n");
	memcpy(&bytes[sizeof(p)],name.data(),p.readHeader.namelen);
	printf("3\n");
	memcpy(&bytes[sizeof(p)+p.readHeader.namelen],&value,p.datalen);
	printf("4\n");
	a.send(req);
	printf("5\n");
	a.disconnect("inproc://mqserver");
	printf("6\n");
	a.close();
	printf("7\n");
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

uint8_t MQServer::parsePacket(zmq::message_t& request, zmq::message_t& reply) {
	if (request.size() > sizeof(packetHeader)) {
		packetHeader* p = (mqserver::packetHeader*) (request.data()); // Packet header is at start of data
		uint8_t* bytes = (uint8_t*) (request.data()); // Lets us access data as a byte array
		auto namelen = p->readHeader.namelen; // Grab the namelength from the packe header
		printf("Packet type %d", p->readHeader.type);
		switch (p->readHeader.type) { // switch over packet type
		case 0: {
			// Read
			std::string name((char*) (&bytes[sizeof(p->readHeader)]), namelen); // Name string starts after read header
			getResponseHeader r;
			r.status = 0; // Nominal status is 0
			r.dataType = p->readHeader.type; //
			switch (p->readHeader.type) {
			case 0: // Null
				return 2;
				break;
			case 1: { // Long
				r.datalen = 8;
				int64_t val = GetLong(name);
				request.rebuild(sizeof(r) + r.datalen);
				memcpy(request.data(), &r, sizeof(r));
				memcpy((char*) request.data() + sizeof(r), &val, sizeof(val));
				break;
			}
			case 2: { // Double
				r.datalen = 8;
				double val = GetDouble(name);
				request.rebuild(sizeof(r) + r.datalen);
				memcpy(request.data(), &r, sizeof(r));
				memcpy((char*) request.data() + sizeof(r), &val, sizeof(val));
				break;
			}
			case 3: { // String
				std::string val = GetString(name);
				r.datalen = val.length();
				request.rebuild(sizeof(r) + r.datalen);
				memcpy(request.data(), &r, sizeof(r));
				memcpy((char*) request.data() + sizeof(r), val.data(),
						r.datalen);
				break;
			}
			default: {
				return 2;
				break;
			}
			}
			break;
		}
		case 1: {
			auto totalLen = sizeof(*p) + p->readHeader.namelen + p->datalen;
			printf("Namelen: %d Datalen: %d Header len: %d Packet Length: %d", p->readHeader.namelen, p->datalen, sizeof(*p), request.size());
			if (totalLen != request.size()) {
				printf("Non matching length Expected: %d, recieved %d", totalLen, request.size());
				return 4;
			}
			// Write
			std::string name((char*) (&bytes[sizeof(*p)]), namelen); // Name string starts after write header
			auto dataOffset = sizeof(*p) + namelen; // Data starts after header and name
			setResponseHeader h;
			h.status = 0;
			switch (p->dataType) {
			case 0: { // Null
				return 2;
				break;
			}
			case 1: { // Long
				printf("1.1\n");
				printf("Data offset %d\n", dataOffset);
				for (int i = 0; i < (dataOffset+8); i++) {
					printf("%x",bytes[i]);
				}
				int64_t val = ((int64_t*)(bytes + dataOffset))[0];
				printf("1.2\n");
				Set(name, val);
				printf("1.3\n");
				reply.rebuild(&h,sizeof(h));
				break;
			}
			case 2: { // Double
				double val = *((double*)(&bytes[dataOffset]));
				printf("Setting %s %d to %f\n", name.c_str(),name.length(), val);
				printf("Total length %d, actual length %d\n", (sizeof(*p) + p->datalen + p->readHeader.namelen),request.size());
				Set(name, val);
				reply.rebuild(&h,sizeof(h));
				break;
			}
			case 3: { // String
				std::string val((char*) &bytes[dataOffset]);
				Set(name, val);
				reply.rebuild(&h,sizeof(h));
				break;
			}
			default: {
				return 2;
				break;
			}
			}
			break;
		}
		default: {
			// Unknown packet type
			return 1;
			break;
		}
		}
	}
	return 0;
}

void MQServer::Run() {
	sock->bind("tcp://*:5810");
	sock->bind("inproc://mqserver"); //bind locally
	while (running) {
		zmq::message_t request(200), reply;
		sock->recv(&request);
		printf("Received packet\n");
		printf("More? %d",request.more());
		try {
		auto error = parsePacket(request, reply);
		printf("Processed packet with error %d\n", error);
		if (error != 0) {
			reply.rebuild(2); // 2 byte error response, type=255, second byte is error code
			((uint8_t*) reply.data())[0] = 255;
			((uint8_t*) reply.data())[1] = error;
		}
		} catch (std::exception ex) {
			printf("CAUGHT EXCEPTION: %s" ,ex.what());
			reply.rebuild(2); // 2 byte error response, type=255, second byte is error code
			((uint8_t*) reply.data())[0] = 255;
			((uint8_t*) reply.data())[1] = 255;
		}
		sock->send(reply);
	}
	sock->close();
}

} /* namespace mqserver */
