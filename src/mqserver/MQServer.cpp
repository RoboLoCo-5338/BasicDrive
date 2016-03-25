/*
 * MQServer.cpp
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#include "MQServer.h"
#include "unistd.h"
#include <sstream>
#include <typeinfo>

//Set Debug status
#define DEBUG 0
#if DEBUG == 1
	#define IFDEBUG(X) {X}
#else
	#define IFDEBUG(X)
#endif

namespace mqserver {

struct packetHeader {
	struct {
		uint8_t type;
		uint16_t namelen;
		uint8_t dataType;
	} readHeader;
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
	zmq::socket_t* a;
	try {
		a = new zmq::socket_t(*context,zmq::socket_type::req);
	} catch (std::exception &e) {
		fprintf(stderr,"Exception in MQServer::PutString ");
		fprintf(stderr,"%s\n",e.what());
		IFDEBUG(fprintf(stderr, "Type: %s", typeid(e).name());)
		return;
	}
	a->connect("inproc://mqserver");
	packetHeader p;
	p.readHeader.dataType = 3; // string
	p.datalen = value.size();
	p.readHeader.type = 1; // put/write
	p.readHeader.namelen = name.size();
	zmq::message_t req(sizeof(packetHeader) + p.datalen + p.readHeader.namelen);
	uint8_t* bytes = (uint8_t*)req.data();
	memcpy(req.data(),&p,sizeof(p));
	memcpy(&bytes[sizeof(p)],name.data(),p.readHeader.namelen);
	memcpy(&bytes[sizeof(p)+p.readHeader.namelen],value.data(),p.datalen);
	a->send(req);
	a->disconnect("inproc://mqserver");
	a->close();
	delete a;
}

void MQServer::PutDouble(std::string name, double value) {
	zmq::socket_t* a;
	try {
		a = new zmq::socket_t(*context,zmq::socket_type::req);
	} catch (std::exception &e) {
		fprintf(stderr,"Exception in MQServer::PutDouble ");
		fprintf(stderr,"%s\n",e.what());
		IFDEBUG(fprintf(stderr, "Type: %s", typeid(e).name());)
		return;
	}
	a->connect("inproc://mqserver");
	packetHeader p;
	p.readHeader.dataType = 2; // double
	p.datalen = 8;
	p.readHeader.type = 1; // put/write
	p.readHeader.namelen = name.size();
	zmq::message_t req(sizeof(packetHeader) + p.datalen + p.readHeader.namelen);
	uint8_t* bytes = (uint8_t*)req.data();
	memcpy(req.data(),&p,sizeof(p));
	memcpy(&bytes[sizeof(p)],name.data(),p.readHeader.namelen);
	memcpy(&bytes[sizeof(p)+p.readHeader.namelen],&value,p.datalen);
	a->send(req);
	a->disconnect("inproc://mqserver");
	a->close();
	delete a;
}

void MQServer::PutLong(std::string name, int64_t value) {
	zmq::socket_t* a;
	try {
		a = new zmq::socket_t(*context,zmq::socket_type::req);
	} catch (std::exception &e) {
		fprintf(stderr,"Exception in MQServer::PutLong ");
		fprintf(stderr,"%s\n",e.what());
		IFDEBUG(fprintf(stderr, "Type: %s", typeid(e).name());)
		return;
	}
	a->connect("inproc://mqserver");
	packetHeader p;
	p.readHeader.dataType = 1; // long
	p.datalen = 8;
	p.readHeader.type = 1; // put/write
	p.readHeader.namelen = name.size();
	zmq::message_t req(sizeof(packetHeader) + p.datalen + p.readHeader.namelen);
	uint8_t* bytes = (uint8_t*)req.data();
	memcpy(req.data(),&p,sizeof(p));
	memcpy(&bytes[sizeof(p)],name.data(),p.readHeader.namelen);
	memcpy(&bytes[sizeof(p)+p.readHeader.namelen],&value,p.datalen);
	a->send(req);
	a->disconnect("inproc://mqserver");
	a->close();
	delete a;
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

uint8_t MQServer::parsePacket(zmq::message_t& request, zmq::message_t& reply)
{
	if (request.size() > sizeof(packetHeader))
	{
		//packetHeader* p = (mqserver::packetHeader*) (request.data()); // Packet header is at start of data
		std::stringstream packetIn(std::string((char*)request.data(),request.size()),std::ios_base::in);
		packetHeader p;
		packetIn.read((char*)&p,sizeof(p.readHeader));
		auto namelen = p.readHeader.namelen; // Grab the namelength from the packet header
		IFDEBUG(fprintf(stderr,"Packet type %d\n", p.readHeader.type);)
		switch (p.readHeader.type)
		{ // switch over packet type
			case 0:
			{
				// Read
				IFDEBUG(printf("GOT READ PACKET\n");)
				std::string name(namelen,'\0');
				packetIn.read((char*)name.data(),namelen); // read name from packet
				//printf("Read name \"%s\"", name);
				getResponseHeader r;
				r.status = 0; // Nominal status is 0
				r.dataType = p.readHeader.dataType; //
				switch (p.readHeader.dataType)
				{
					case 0: // Null
						return 2;
						break;
					case 1: { // Long
						r.datalen = 8;
						int64_t val = GetLong(name);
						fprintf(stderr,"Sending long \"%s\"=%ld\n", name.c_str(), val);
						reply.rebuild(sizeof(r) + r.datalen);
						IFDEBUG(fprintf(stderr, "Sizeof %d\n", sizeof(r) + r.datalen);)
						memcpy(reply.data(), &r, sizeof(getResponseHeader));
						memcpy((char*) reply.data() + sizeof(r), &val, sizeof(val));
						IFDEBUG(fprintf(stderr, "Message size %d\n", reply.size());)
						break;
					}
					case 2:
					{ // Double
						r.datalen = 8;
						double val = GetDouble(name);
						fprintf(stderr,"Sending double \"%s\"=%f\n", name.c_str(), val);
						reply.rebuild(sizeof(r) + r.datalen);
						IFDEBUG(fprintf(stderr, "Sizeof %d\n", sizeof(r) + r.datalen);)
						memcpy(reply.data(), &r, sizeof(r));
						memcpy((char*) reply.data() + sizeof(r), &val, sizeof(val));
						IFDEBUG(fprintf(stderr, "Message size %d\n", reply.size());)
						break;
					}
					case 3:
					{ // String
						std::string val = GetString(name);
						fprintf(stderr,"Sending string \"%s\"=\"%s\"\n", name.c_str(), val.c_str());
						r.datalen = val.length();
						reply.rebuild(sizeof(r) + r.datalen);
						IFDEBUG(fprintf(stderr, "Sizeof %s\n", sizeof(r) + r.datalen);)
						memcpy(reply.data(), &r, sizeof(r));
						memcpy((char*) reply.data() + sizeof(r), val.data(), r.datalen);
						IFDEBUG(fprintf(stderr, "Message size %d\n", reply.size());)
						break;
					}
					default:
					{
						return 2;
						break;
					}
				}
				break;
			}
			case 1:
			{
				packetIn.read(&((char*)&p)[sizeof(p.readHeader)],sizeof(p)-sizeof(p.readHeader)); // read the rest of the packet
				auto totalLen = sizeof(p) + p.readHeader.namelen + p.datalen;
				if (totalLen != request.size())
				{
					IFDEBUG(fprintf(stderr,"Non matching length Expected: %d, recieved %d", totalLen, request.size());)
					return 4;
				}
				// Write
				std::string name(namelen,'\0');
				packetIn.read((char*)name.data(),namelen);
				setResponseHeader h;
				h.status = 0;
				switch (p.readHeader.dataType)
				{
					case 0:
					{ // Null
						return 2;
						break;
					}
					case 1: { // Long
						int64_t val;
						packetIn.read((char*)&val,sizeof(val));
						IFDEBUG(fprintf(stderr,"Setting long \"%s\" = %ld\n",name.c_str(), val);)
						Set(name, val);
						reply.rebuild(&h,sizeof(h));
						break;
					}
					case 2:
					{ // Double
						double val;
						packetIn.read((char*)&val,sizeof(val));
						IFDEBUG(fprintf(stderr,"Setting double \"%s\" = %f\n",name.c_str(), val);)
						Set(name, val);
						reply.rebuild(&h,sizeof(h));
						break;
					}
					case 3:
					{ // String
						std::string val(p.datalen,'\0');
						packetIn.read((char*)val.data(),val.length());
						IFDEBUG(fprintf(stderr,"Setting string \"%s\" = \"%s\"\n",name.c_str(), val.c_str());)
						Set(name, val);
						reply.rebuild(&h,sizeof(h));
						break;
					}
					default:
					{
						return 2;
						break;
					}
				}
				break;
			}
			default:
			{
				// Unknown packet type
				return 1;
				break;
			}
		}
	}
	IFDEBUG(fprintf(stderr, "SIZE OF PACKET: %d\n", reply.size());)
	return 0;
}

void MQServer::Run() {
	sock->bind("tcp://*:5810");
	sock->bind("inproc://mqserver"); //bind locally
	while (running) {
		zmq::message_t request(200), reply;
		sock->recv(&request);
		try {
		auto error = parsePacket(request, reply);
		if (error != 0) {
			fprintf(stderr, "Error code in parsing: %d\n", error);
			reply.rebuild(2); // 2 byte error response, type=255, second byte is error code
			((uint8_t*) reply.data())[0] = 255;
			((uint8_t*) reply.data())[1] = error;
		}
	} catch (std::exception &e) {
			fprintf(stderr,"Exception in MQServer::Run() ");
			fprintf(stderr, "%s\n", e.what());
			IFDEBUG(fprintf(stderr, "Type: %s", typeid(e).name());)
			reply.rebuild(2); // 2 byte error response, type=255, second byte is error code
			((uint8_t*) reply.data())[0] = 255;
			((uint8_t*) reply.data())[1] = 255;
		}
		IFDEBUG(fprintf(stderr, "Size: %d\n", reply.size());)
		sock->send(reply);
	}
	sock->close();
}

void MQServer::Start() {
	if (!running) {
		running = true;
		std::thread newThread(&MQServer::Run, this);
		this->serverThread.swap(newThread);
	}
}

void MQServer::SendStop() {
	zmq::socket_t* a;
	retry:
	try {
		a = new zmq::socket_t(*context,zmq::socket_type::req);
	} catch (std::exception &e) {
		fprintf(stderr,"Exception in MQServer::SendStop() ");
		fprintf(stderr,"%s\n",e.what());
		IFDEBUG(fprintf(stderr, "Type: %s", typeid(e).name());)
		sleep(1);
		goto retry;
	}
	//zmq::socket_t a(*context,zmq::socket_type::req);
	a->connect("inproc://mqserver");
	zmq::message_t req(0);
	a->send(req);
	a->disconnect("inproc://mqserver");
	a->close();
	delete a;
}

void MQServer::Stop() {
	if (running) {
		running = false;
		SendStop();
		this->serverThread.join();
	}
}

} /* namespace mqserver */
