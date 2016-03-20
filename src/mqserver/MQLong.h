/*
 * MQLong.h
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#ifndef SRC_MQSERVER_MQLONG_H_
#define SRC_MQSERVER_MQLONG_H_

#include "MQObject.h"

namespace mqserver {

class MQLong: public MQObject {
	long value;
public:
	long Get();
	MQLong(long val);
	virtual ~MQLong();
};

} /* namespace mqserver */

#endif /* SRC_MQSERVER_MQLONG_H_ */
