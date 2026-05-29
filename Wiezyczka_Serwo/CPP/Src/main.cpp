//Author note:
//Its project for my university class, but I'll do comments in english cause i would like to share
//this project in my portfolio6
#include "main.h" //to solve (unprofessional)
#include "NRF24.h"
#include "Laser.h"
#include "Husky.h"
#include <cstdio>
#include <cstring>
extern "C" {
    #include "CppMain.h"
}

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi1;
extern I2C_HandleTypeDef hi2c1;


struct __attribute__((packed)) DataPacket {
    int16_t speedX;
    int16_t speedY;
    int16_t laserOnOff;
    int16_t laserShot;
    int16_t modeStm;
    int16_t modeServo;
};


// Adress on our singleton
NRF24& radio = NRF24::getInstance();
extern "C" void CppMain(void) {

	// Channel_2 and 3 its our pins for servos. I using RC 360 servos 996
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);


    //Sending msg to stm32 console (we can hear this from our Computer by USB connection
    char startMsg[] = "\r\n SYSTEM STM32 (C++) \r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)startMsg, strlen(startMsg), 100);


    //Our canal of radio waves
    uint8_t AdresOdbiorczy[5] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE1};

    radio.Init();
    radio.RxMode(AdresOdbiorczy, 115);

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1600);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1600);
    HAL_Delay(1000);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1500);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1500);



    uint32_t lastHeartbeat = HAL_GetTick();
    int16_t pulseXChanged=0;
    int16_t pulseYChanged=0;
    float currentPosX = 1500.0f;
    float currentPosY = 1500.0f;
    Laser laser(GPIOA, GPIO_PIN_8);
    Husky camera(&hi2c1);
    if(camera.ping()) {
        char msg[] = "Kamera HuskyLens gotowa!\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    }
    while (1) {
    	if (HAL_GetTick() - lastHeartbeat > 2000) {
    	            char diagMsg[] = "STM32: Nasluchuje eteru...\r\n";
    	            HAL_UART_Transmit(&huart2, (uint8_t*)diagMsg, strlen(diagMsg), 100);
    	            lastHeartbeat = HAL_GetTick();
    	        }
    	/*DataPacket odebraneDane1;
    	        	char sizeMsg[50];
    	        	sprintf(sizeMsg, "Rozmiar paczki: %u\r\n", sizeof(DataPacket));
    	        	HAL_UART_Transmit(&huart2, (uint8_t*)sizeMsg, strlen(sizeMsg), 100);

        uint8_t status;
        uint8_t reg = 0xFF;
        radio.CSN_Select();
        HAL_SPI_TransmitReceive(&hspi1, &reg, &status, 1, 100);
        radio.CSN_UnSelect();
        char diagMsg[50];
        sprintf(diagMsg, "STATUS: 0x%02X\r\n", status);
        HAL_UART_Transmit(&huart2, (uint8_t*)diagMsg, strlen(diagMsg), 100);
        HAL_Delay(1000);
		*/

        // isDataAvailable -> radio looking for some packages in air
        if (radio.IsDataAvailable()) {

        	//int16- 16bits - we are sure how huge is int, standard int could be 16-32bits, less memory lose
        	DataPacket odebraneDane;
        	char sizeMsg[50];
        	sprintf(sizeMsg, "Rozmiar paczki: %u\r\n", sizeof(DataPacket));
        	HAL_UART_Transmit(&huart2, (uint8_t*)sizeMsg, strlen(sizeMsg), 100);

            radio.Receive((int*)&odebraneDane);
            laser.update();


            if (odebraneDane.modeStm == 0) {
            	char sizeMsg2[50];
            	        	sprintf(sizeMsg2, "Tryb WSAD", sizeof(DataPacket));
            	        	HAL_UART_Transmit(&huart2, (uint8_t*)sizeMsg2, strlen(sizeMsg2), 100);
            int16_t pulseX = odebraneDane.speedX;
            int16_t pulseY = odebraneDane.speedY;


            // Sending msg to STM32 console
            char wiadomosc[64];
            sprintf(wiadomosc, "Odebrano PWM X: %u, Y: %u \n", pulseX, pulseY);
            HAL_UART_Transmit(&huart2, (uint8_t*)wiadomosc, strlen(wiadomosc), 100);


            // We dont want to servos break the cables
            if (pulseX < 500) pulseX = 500;
            if (pulseX > 2500) pulseX = 2500;
            if (pulseY < 1000) pulseY = 1000;
            if (pulseY > 2000) pulseY = 2000;

            if (odebraneDane.laserShot == 1) laser.shoot();
            currentPosX = pulseX;
            currentPosY = pulseY;

            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulseY);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulseX);
            }
            // Servo move
//            if(pulseX != pulseXChanged){
//            pulseXChanged = pulseX;
//            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulseX);
//            }
//            if(pulseY != pulseYChanged){
//            pulseYChanged = pulseY;
//            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulseY);
            else if (odebraneDane.modeStm == 1) {
          	HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, 0x64, 3, 100);
        	char sizeMsg3[50];
            	        	sprintf(sizeMsg3, "Tryb Auto", sizeof(DataPacket));
            	        	HAL_UART_Transmit(&huart2, (uint8_t*)sizeMsg3, strlen(sizeMsg3), 100);
        	TrackingObj cel = camera.getTrackedObject();
        	if (cel.isDet) {
        	    int martwa_strefa = 20;
        	    int krok_serwa = 15;

        	    if (cel.xCenter > (160 + martwa_strefa)) {
        	        currentPosX -= krok_serwa;
        	    }
        	    else if (cel.xCenter < (160 - martwa_strefa)) {
        	        currentPosX += krok_serwa;
        	    }


        	    if (cel.yCenter > (120 + martwa_strefa)) {
        	        currentPosY -= krok_serwa;
        	    }
        	    else if (cel.yCenter < (120 - martwa_strefa)) {
        	        currentPosY += krok_serwa;
        	    }

        	    if (currentPosX < 500) currentPosX = 500;
        	    if (currentPosX > 2500) currentPosX = 2500;
        	    if (currentPosY < 1000) currentPosY = 1000;
        	    if (currentPosY > 2000) currentPosY = 2000;


        	    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (uint16_t)currentPosX);
        	    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint16_t)currentPosY);
        	     laser.shoot();
        	}
            }

            else if (odebraneDane.modeStm == 2) {

            }
//            if(status == HAL_OK) {
//            	laser.shoot();
//            }

            }
    }
}
