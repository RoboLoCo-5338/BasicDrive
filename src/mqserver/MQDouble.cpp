/*
 * MQDouble.cpp
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#include <mqserver/MQDouble.h>

namespace mqserver {

double MQDouble::Get() {
	return value;
}

MQDouble::MQDouble(double val) {
	value = val;
	// TODO Auto-generated constructor stub

}

MQDouble::~MQDouble() {
	// TODO Auto-generated destructor stub
}

} /* namespace mqserver */
