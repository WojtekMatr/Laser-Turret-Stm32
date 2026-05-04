/*
 * NRF24.cpp
 *
 *  Created on: Mar 9, 2026
 *      Author: dom12
 */
#include "NRF24.h"

extern "C" {
    #include "main.h"
}
extern SPI_HandleTypeDef hspi1;

NRF24::NRF24() {
}

NRF24::~NRF24() {
}
void NRF24::CSN_Select(void) {
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
}
void NRF24::CSN_UnSelect(void) {
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);
}
void NRF24::CE_Enable(void) {
    HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_SET);
}
void NRF24::CE_Disable(void) {
    HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET);
}

void NRF24::WriteReg(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg | (1 << 5);
    buf[1] = data;

    CSN_Select();
    HAL_SPI_Transmit(&hspi1, buf, 2, 100);
    CSN_UnSelect();
}


void NRF24::Init(void) {
    CE_Disable();
    WriteReg(0x00, 0x08);
    WriteReg(0x01, 0x00);
    WriteReg(0x06, 0x07);
    WriteReg(0x11, 12);
    CE_Enable();
}

void NRF24::RxMode(uint8_t *Address, uint8_t Channel) {
    CE_Disable();
    WriteReg(0x05, Channel);

    CSN_Select();
    uint8_t reg = 0x0A | 0x20;
    HAL_SPI_Transmit(&hspi1, &reg, 1, 100);
    HAL_SPI_Transmit(&hspi1, Address, 5, 100);
    CSN_UnSelect();

    WriteReg(0x00, 0x0F);
    HAL_Delay(2);
    FlushRX();
    CE_Enable();
}

uint8_t NRF24::IsDataAvailable(void) {
    uint8_t status;
    uint8_t reg = 0xFF;

    CSN_Select();
    HAL_SPI_TransmitReceive(&hspi1, &reg, &status, 1, 100);
    CSN_UnSelect();

    if ((status & (1 << 6))) return 1;
    return 0;
}

void NRF24::Receive(int *data) {
    uint8_t cmd = 0x61;

    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    HAL_SPI_Receive(&hspi1, (uint8_t *)data, 12, 100);
    CSN_UnSelect();

    WriteReg(0x07, 0x40);
}
void NRF24::FlushRX(void) {
    uint8_t cmd = 0xE2;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    CSN_UnSelect();
}




