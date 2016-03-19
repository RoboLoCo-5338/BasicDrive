/*
 * Scissor.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: RoboLoCo
 */

#include <Scissor.h>

Scissor::Scissor(shared_ptr<SpeedController> l, shared_ptr<SpeedController> r,
		shared_ptr<DigitalInput> limit_left_up, shared_ptr<DigitalInput> limit_right_up) :
		left(l), right(r), limitleftup(limit_left_up), limitrightup(limit_right_up) {

}

Scissor::~Scissor() {
	// TODO Auto-generated destructor stub
}

void Scissor::Set(float speed) {
	if (speed > 0) {
		if (limitleftup->Get()) {
			left->Set(speed);
		} else {
			left->Set(0);
		}
		if (limitrightup->Get()) {
			right->Set(speed);
		} else {
			right->Set(0);
		}
	} else if (speed < 0) {
		left->Set(speed);
		right->Set(speed);
	} else {
		left->Set(0);
		right->Set(0);
	}
}
