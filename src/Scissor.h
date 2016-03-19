/*
 * Scissor.h
 *
 *  Created on: Feb 23, 2016
 *      Author: RoboLoCo
 */

#ifndef SRC_SCISSOR_H_
#define SRC_SCISSOR_H_

#include <memory>
#include "WPILib.h"

using std::shared_ptr;

class Scissor {
	shared_ptr<SpeedController> left, right;
	shared_ptr<DigitalInput> limitleftup, limitrightup;
public:
	Scissor(shared_ptr<SpeedController> l, shared_ptr<SpeedController> r,
			shared_ptr<DigitalInput> limit_left_up, shared_ptr<DigitalInput> limit_right_up);
	virtual ~Scissor();
	void Set(float speed);
};

#endif /* SRC_SCISSOR_H_ */
