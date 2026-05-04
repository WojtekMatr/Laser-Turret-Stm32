/*
 * Laser.cpp
 *
 * Created on: Apr 13, 2026
 * Author: dom12
 */

#include "Laser.h"
Laser::Laser(GPIO_TypeDef* port, uint16_t pin) {
    GPIOx = port;
    GPIO_Pin = pin;
    startTime = 0;
    isFiring = false;
}
Laser::~Laser() {
    // TODO Auto-generated destructor stub
}
void Laser::shoot() {
    if (!isFiring) {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
        startTime = HAL_GetTick();
        isFiring = true;
    }
}

void Laser::update() {
    if (isFiring && (HAL_GetTick() - startTime >= 500)) {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
        isFiring = false;
    }
}
