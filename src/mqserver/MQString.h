/*
 * MQString.h
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#ifndef SRC_MQSERVER_MQSTRING_H_
#define SRC_MQSERVER_MQSTRING_H_

#include "string"

namespace mqserver {

class MQString {
	std::string value;
public:
	std::string Get();
	MQString(std::string val);
	virtual ~MQString();
};

} /* namespace mqserver */

#endif /* SRC_MQSERVER_MQSTRING_H_ */
