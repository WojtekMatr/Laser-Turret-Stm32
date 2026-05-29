/*
 * Husky.cpp
 *
 *  Created on: May 4, 2026
 *      Author: dom12
 */

#include <Husky.h>

Husky::Husky(I2C_HandleTypeDef* i2c_handle) {
	// TODO Auto-generated constructor stub
	hi2c = i2c_handle;
}
bool Husky::ping() {
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(hi2c, I2C_ADDRESS, 3, 100);
    return (status == HAL_OK);
}
TrackingObj Husky::getTrackedObject() {
    // Domyślnie obiekt NIE jest wykryty (same zera)
    TrackingObj obj = {false, 0, 0, 0, 0};

    uint8_t cmd[6] = {0x55, 0xAA, 0x11, 0x00, 0x20, 0x30};

    HAL_I2C_Master_Transmit(hi2c, I2C_ADDRESS, cmd, 6, 10);

    HAL_Delay(5);

    uint8_t buffer[32] = {0};
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(hi2c, I2C_ADDRESS, buffer, 32, 50);

    if (status == HAL_OK) {

        for (int i = 0; i < 32 - 15; i++) {

            if (buffer[i] == 0x55 && buffer[i+1] == 0xAA && buffer[i+4] == 0x2A) {


                obj.xCenter = (buffer[i+6] << 8) | buffer[i+5];
                obj.yCenter = (buffer[i+8] << 8) | buffer[i+7];
                obj.width   = (buffer[i+10] << 8) | buffer[i+9];
                obj.height  = (buffer[i+12] << 8) | buffer[i+11];
                obj.isDet = true;

                break;
            }
        }
    }

    return obj;
}
Husky::~Husky() {
	// TODO Auto-generated destructor stub
}

