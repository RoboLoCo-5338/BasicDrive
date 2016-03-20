/*
 * MQString.cpp
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#include <mqserver/MQString.h>

namespace mqserver {

std::string MQString::Get() {
	return value;
}

MQString::MQString(std::string val) {
	value = val;
	// TODO Auto-generated constructor stub

}

MQString::~MQString() {
	// TODO Auto-generated destructor stub
}

} /* namespace mqserver */
