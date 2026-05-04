/*
 * NRF24.h
 *
 *  Created on: Mar 9, 2026
 *      Author: dom12
 */

#ifndef NRF24_H_
#define NRF24_H_

#include <cstdint>
#include "stm32f4xx_hal.h"
//we declaring class in header and in cpp we just adds methods NRF::
//its standard for creating of objects in new cpp projects
class NRF24 {
public:
	NRF24(const NRF24 &other) = delete;
	NRF24& operator=(const NRF24&) = delete;
	static NRF24& getInstance() {
	        static NRF24 instance;
	        return instance;
	    }
	void Init();
    void RxMode(uint8_t *Address, uint8_t Channel);
    uint8_t IsDataAvailable();
    void Receive(int *data);
	void CSN_Select();
	void CSN_UnSelect();




private:
	NRF24();
	~NRF24();

	void CE_Enable();
    void CE_Disable();
    void WriteReg(uint8_t reg, uint8_t data);
    void FlushRX();
};

#endif /* NRF24_H_ */
