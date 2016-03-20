/*
 * MQLong.cpp
 *
 *  Created on: Mar 20, 2016
 *      Author: RoboLoCo
 */

#include <mqserver/MQLong.h>

namespace mqserver {

long MQLong::Get() {
	return value;
}

MQLong::MQLong(long val) {
	value = val;
	// TODO Auto-generated constructor stub

}

MQLong::~MQLong() {
	// TODO Auto-generated destructor stub
}

} /* namespace mqserver */
