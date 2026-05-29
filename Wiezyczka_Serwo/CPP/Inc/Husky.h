/*
 * Husky.h
 *
 *  Created on: May 4, 2026
 *      Author: dom12
 */

#ifndef INC_HUSKY_H_
#define INC_HUSKY_H_
#include "main.h"
#include <cstdint>
struct TrackingObj {
    bool isDet;
    int16_t xCenter;
    int16_t yCenter;
    int16_t width;
    int16_t height;
};

class Husky {
private:
		I2C_HandleTypeDef* hi2c;
	    static const uint16_t I2C_ADDRESS = 0x64;
	    uint8_t rxBuffer[16];
	    bool validateChecksum(uint8_t* data, int length);
public:
	Husky();
	Husky(I2C_HandleTypeDef* i2c_handle);
	bool ping();
	TrackingObj getTrackedObject();
	virtual ~Husky();
};

#endif /* INC_HUSKY_H_ */
