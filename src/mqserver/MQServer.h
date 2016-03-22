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

namespace mqserver {

// MQServer stores and receives values from co-processors in a key-value store
class MQServer {
	std::shared_ptr<zmq::context_t> context;
	std::shared_ptr<zmq::socket_t> sock;
	std::thread serverThread;
	std::map<std::string, std::string> stringMap{};
	std::map<std::string, double> doubleMap{};
	std::map<std::string, long> longMap{};

	bool running = true;

	void Run();
public:
	std::string GetString(std::string name);
	double GetDouble(std::string name);
	long GetLong(std::string name);
	void Set(std::string name, std::string value);
	void Set(std::string name, double value);
	void Set(std::string name, long value);
	MQServer();
	virtual ~MQServer() = default;
};

} /* namespace mqserver */

#endif /* SRC_MQSERVER_MQSERVER_H_ */
