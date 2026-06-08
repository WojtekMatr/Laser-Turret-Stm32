/*
 * Vl53l1platform.h
 *
 *  Created on: Jun 7, 2026
 *      Author: dom12
 */

#ifndef _VL53L1_PLATFORM_H_
#define _VL53L1_PLATFORM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int8_t VL53L1_WriteMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count);
int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count);
int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data);
int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data);
int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data);
int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data);
int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data);
int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data);
int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms);

#ifdef __cplusplus
}
#endif

#endif
