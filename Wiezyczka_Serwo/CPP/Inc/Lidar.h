/*
 * Lidar.h
 *
 *  Created on: Jun 7, 2026
 *      Author: dom12
 */

#ifndef LIDAR_H
#define LIDAR_H

#include "main.h"

class Lidar {
private:
    uint16_t dev;
public:
    Lidar();
    bool init();
    uint16_t getDistance();
};

#endif/* INC_LIDAR_H_ */
