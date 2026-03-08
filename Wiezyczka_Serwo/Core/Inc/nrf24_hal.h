#ifndef NRF24_HAL_H_
#define NRF24_HAL_H_

#include "stm32f4xx_hal.h"

void NRF24_Init(void);
void NRF24_RxMode(uint8_t *Address, uint8_t Channel);
uint8_t NRF24_IsDataAvailable(void);
void NRF24_Receive(int *data);

#endif
