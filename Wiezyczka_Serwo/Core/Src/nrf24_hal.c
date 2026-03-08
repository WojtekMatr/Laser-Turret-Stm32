#include "nrf24_hal.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;

// Funkcje pomocnicze do sterowania pinami CSN i CE
void CSN_Select (void) { HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET); }
void CSN_UnSelect (void) { HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET); }
void CE_Enable (void) { HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_SET); }
void CE_Disable (void) { HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET); }

void NRF24_WriteReg(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg | 1 << 5; // Rejestr zapisu
    buf[1] = data;
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, buf, 2, 100);
    CSN_UnSelect();
}

void NRF24_Init(void) {
    CE_Disable();
    NRF24_WriteReg(0x00, 0x08); // Konfiguracja podstawowa
    NRF24_WriteReg(0x01, 0x00); // Wylacz auto-ack (dla uproszczenia)
    NRF24_WriteReg(0x06, 0x07); // 1 Mbps, moc 0dBm
    NRF24_WriteReg(0x11, 4);    // Szerokosc danych: 4 bajty (dla int)
    CE_Enable();
}

void NRF24_RxMode(uint8_t *Address, uint8_t Channel) {
    CE_Disable();
    NRF24_WriteReg(0x05, Channel); // Ustawienie kanalu

    CSN_Select();
    uint8_t reg = 0x0A | 0x20; // W_REGISTER (Zapis)
    HAL_SPI_Transmit(&hspi1, &reg, 1, 100);
    HAL_SPI_Transmit(&hspi1, Address, 5, 100);
    CSN_UnSelect();

    NRF24_WriteReg(0x00, 0x0F); // Power Up + RX Mode

    // --- KRYTYCZNA LINIJKA ---
    HAL_Delay(2); // Dajemy radiu 5 milisekund na ustabilizowanie kwarcu!

    CE_Enable();
}

uint8_t NRF24_IsDataAvailable(void) {
    uint8_t status;
    uint8_t reg = 0xFF; // NOP - najbezpieczniejsza komenda do odczytu statusu
    CSN_Select();
    HAL_SPI_TransmitReceive(&hspi1, &reg, &status, 1, 100);
    CSN_UnSelect();
    if ((status & (1 << 6))) return 1; // Sprawdzamy bit RX_DR
    return 0;
}

void NRF24_Receive(int *data) {
    uint8_t cmd = 0x61; // Komenda Read Payload
    CSN_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    HAL_SPI_Receive(&hspi1, (uint8_t *)data, 4, 100);
    CSN_UnSelect();
    NRF24_WriteReg(0x07, 0x40); // Czyscimy flage odebrania danych
}
