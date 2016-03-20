/*
 * MQServer.h
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#ifndef SRC_MQSERVER_MQSERVER_H_
#define SRC_MQSERVER_MQSERVER_H_

#include "zmq.hpp"
#include <thread>
#include <memory>
#include <string>
#include <map>
#include "MQObject.h"

namespace mqserver {

// MQServer stores and receives values from co-processors in a key-value store
class MQServer {
	std::shared_ptr<zmq::context_t> context;
	std::shared_ptr<zmq::socket_t> sock;
	std::thread serverThread;
	std::map<std::string, MQObject> map;

	bool running = true;

	void Run();
	void Set(std::string key, MQObject&);
	MQObject* Get(std::string);
public:
	MQServer();
	virtual ~MQServer() = default;
};

} /* namespace mqserver */

#endif /* SRC_MQSERVER_MQSERVER_H_ */
