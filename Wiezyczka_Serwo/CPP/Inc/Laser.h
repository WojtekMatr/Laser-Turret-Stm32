/*
 * Laser.h
 *
 *  Created on: Apr 13, 2026
 *      Author: dom12
 */

#ifndef INC_LASER_H_
#define INC_LASER_H_

#include "main.h"

class Laser {
private:
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	uint32_t startTime;
	bool isFiring;
public:
	Laser(GPIO_TypeDef* port, uint16_t pin);
	virtual ~Laser();
	void shoot();
	void update();
};

#endif /* INC_LASER_H_ */
