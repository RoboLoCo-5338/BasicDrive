/*
 * MQObject.h
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#ifndef SRC_MQSERVER_MQOBJECT_H_
#define SRC_MQSERVER_MQOBJECT_H_

namespace mqserver {

enum MQObject_type {
	EMPTY = 0,
	LONG = 1,
	DOUBLE = 2,
	STRING = 3,
};

class MQObject {
	MQObject_type type;
public:
	MQObject_type GetType();
	MQObject();
	virtual ~MQObject();
};

} /* namespace mqserver */

#endif /* SRC_MQSERVER_MQOBJECT_H_ */
