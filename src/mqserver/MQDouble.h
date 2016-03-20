/*
 * MQDouble.h
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#ifndef SRC_MQSERVER_MQDOUBLE_H_
#define SRC_MQSERVER_MQDOUBLE_H_

namespace mqserver {

class MQDouble {
	double value;
public:
	double Get();
	MQDouble(double val);
	virtual ~MQDouble();
};

} /* namespace mqserver */

#endif /* SRC_MQSERVER_MQDOUBLE_H_ */
