/*
 * Lidar.cpp
 *
 *  Created on: Jun 7, 2026
 *      Author: dom12
 */
/*
 * Lidar.cpp
 *
 * Created on: Jun 7, 2026
 * Author: dom12
 */

#include "Lidar.h"
extern "C" {
    #include "VL53L1X_api.h"
}

Lidar::Lidar() {
    dev = 0x52; // Domyślny adres I2C dla VL53L1X
}

bool Lidar::init() {
    uint8_t state = 0;
    uint32_t startWait = HAL_GetTick();
    while(state == 0) {
        VL53L1X_BootState(dev, &state);

        if (HAL_GetTick() - startWait > 500) {
            return false;
        }
        HAL_Delay(2);
    }
    VL53L1X_SensorInit(dev);
    VL53L1X_SetDistanceMode(dev, 2);
    VL53L1X_SetTimingBudgetInMs(dev, 33);
    VL53L1X_SetInterMeasurementInMs(dev, 33);
    VL53L1X_StartRanging(dev);
    return true;
}

uint16_t Lidar::getDistance() {
    uint8_t dataReady = 0;
    uint32_t startWait = HAL_GetTick();
    while (dataReady == 0) {
        VL53L1X_CheckForDataReady(dev, &dataReady);
        if (HAL_GetTick() - startWait > 100) {
            return 8888;
        }

        HAL_Delay(1);
    }

    uint16_t distance = 0;
    VL53L1X_GetDistance(dev, &distance);
    VL53L1X_ClearInterrupt(dev);

    return distance;
}
