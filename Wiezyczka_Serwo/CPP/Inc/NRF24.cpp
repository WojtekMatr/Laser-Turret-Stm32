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
    HAL_Delay(5);

    WriteReg(0x01, 0x00);
    WriteReg(0x04, 0x00);
    WriteReg(0x1C, 0x00);
    WriteReg(0x1D, 0x00);

    WriteReg(0x06, 0x07);
    WriteReg(0x11, 12);
    WriteReg(0x00, 0x0F);
    WriteReg(0x07, 0x70);



    uint8_t flush_tx_cmd = 0xE1;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &flush_tx_cmd, 1, 100);
    CSN_UnSelect();

    uint8_t flush_rx_cmd = 0xE2;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &flush_rx_cmd, 1, 100);
    CSN_UnSelect();

    //WriteReg(0x07, 0x70);
    //WriteReg(0x00, 0x0F);
    //WriteReg(0x01, 0x00);

    // TO ZGINĘŁO: Zabicie "duchów" w rejestrach po starym trybie dynamicznym
    //WriteReg(0x1C, 0x00);
    //WriteReg(0x1D, 0x00);

    //WriteReg(0x06, 0x07);
    //WriteReg(0x11, 12);

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
    WriteReg(0x02, 0x01);
    WriteReg(0x11, 12);
    WriteReg(0x07, 0x70);
    WriteReg(0x00, 0x0F);
    uint8_t flush = 0xE2;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &flush, 1, 100);
    CSN_UnSelect();
    HAL_Delay(2);
    //FlushRX();
    CE_Enable();
}

uint8_t NRF24::IsDataAvailable(void) {
    uint8_t status;
    uint8_t reg = 0xFF;

    CSN_Select();
    HAL_SPI_TransmitReceive(&hspi1, &reg, &status, 1, 100);
    CSN_UnSelect();

    if ((status & (1 << 6)) || ((status & 0x0E) != 0x0E)) {
        return 1;
    }
    return 0;
}

void NRF24::Receive(int *data) {
    uint8_t cmd = 0x61;

    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    HAL_SPI_Receive(&hspi1, (uint8_t *)data, 12, 100);
    CSN_UnSelect();


    WriteReg(0x07, 0x70);
}
void NRF24::FlushRX(void) {
    uint8_t cmd = 0xE2;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    CSN_UnSelect();
}
void NRF24::TxMode(uint8_t *Address, uint8_t Channel) {
    CE_Disable();
    WriteReg(0x05, Channel);

    // Zapisz Adres Nadawczy
    CSN_Select();
    uint8_t reg = 0x10 | 0x20;
    HAL_SPI_Transmit(&hspi1, &reg, 1, 100);
    HAL_SPI_Transmit(&hspi1, Address, 5, 100);
    CSN_UnSelect();

    // Wpisz Adres Nadawczy również do P0 (Dobre praktyki sprzętowe nRF24)
    CSN_Select();
    uint8_t reg_rx = 0x0A | 0x20;
    HAL_SPI_Transmit(&hspi1, &reg_rx, 1, 100);
    HAL_SPI_Transmit(&hspi1, Address, 5, 100);
    CSN_UnSelect();

    WriteReg(0x02, 0x01); // Włącz Pipe 0
    WriteReg(0x11, 12);   // Payload na 12 bajtów
    WriteReg(0x07, 0x70); // Posprzątaj WSZYSTKIE sprzętowe błędy!

    // PWR_UP=1, CRCO=1, EN_CRC=1, PRIM_RX=0 (TX MODE)
    WriteReg(0x00, 0x0E);

    // Wyczyść ewentualne śmieci z bufora nadawczego
    uint8_t flush_tx = 0xE1;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &flush_tx, 1, 100);
    CSN_UnSelect();

    HAL_Delay(2); // Daj układowi czas na zmianę trybu napięciowego
}
void NRF24::Transmit(uint8_t *data, uint8_t size) {
    // 1. Ostatnie sprzątanie przed strzałem
    WriteReg(0x07, 0x70);

    // 2. Wypchnięcie ewentualnych śmieci
    uint8_t flush_tx_cmd = 0xE1;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &flush_tx_cmd, 1, 100);
    CSN_UnSelect();

    // 3. Wrzucenie paczki do pamięci FIFO
    uint8_t cmd = 0xA0;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    HAL_SPI_Transmit(&hspi1, data, size, 100);
    CSN_UnSelect();

    // 4. Fizyczny strzał (włączamy wzmacniacz)
    CE_Enable();

    // 5. Czekamy na sygnał sukcesu (TX_DS) lub błędu (MAX_RT) z radia
    uint32_t start = HAL_GetTick();
    uint8_t status = 0;
    while(HAL_GetTick() - start < 15) {
        uint8_t nop = 0xFF;
        CSN_Select();
        HAL_SPI_TransmitReceive(&hspi1, &nop, &status, 1, 100);
        CSN_UnSelect();

        // 0x20 = Wysłano pomyślnie. 0x10 = Odrzucono (przekroczono limit)
        if((status & 0x20) || (status & 0x10)) {
            break;
        }
    }

    // 6. Koniec strzału, gasimy antenę
    CE_Disable();

    // 7. Zresetowanie sprzętu po wystrzale
    WriteReg(0x07, 0x70);
}
void NRF24::EnableAckPayload(void) {
    CE_Disable();
    uint8_t act_cmd = 0x50;
    uint8_t act_data = 0x73;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &act_cmd, 1, 100);
    HAL_SPI_Transmit(&hspi1, &act_data, 1, 100);
    CSN_UnSelect();

    WriteReg(0x01, 0x3F);
    WriteReg(0x1C, 0x3F);
    WriteReg(0x1D, 0x06);
    CE_Enable();
}

void NRF24::WriteAckPayload(uint8_t pipe, uint8_t *data, uint8_t size) {
    uint8_t cmd = 0xA8 | (pipe & 0x07);
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    HAL_SPI_Transmit(&hspi1, data, size, 100);
    CSN_UnSelect();
}
void NRF24::EnableAutoAck(void) {
    WriteReg(0x01, 0x01);
    WriteReg(0x11, 12);
}


