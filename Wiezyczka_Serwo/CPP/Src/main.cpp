//Author note:
//Its project for my university class, but I'll do comments in english cause i would like to share
//this project in my portfolio6
#include "main.h" //to solve (unprofessional)
#include "NRF24.h"
#include "Laser.h"
#include "Husky.h"
#include "Lidar.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
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

struct __attribute__((packed)) STMPacket {
    int16_t stmX;
    int16_t stmY;
    int16_t huskyX;
    int16_t huskyY;
    uint8_t obiektWykryty;
    uint8_t laserStrzelil;
    int16_t padding;
};
struct MapPoint {
    int16_t x;
    int16_t y;
    uint16_t distance;
};
const int MAX_MAP_SIZE = 1200;
MapPoint lidarMap[MAX_MAP_SIZE];
int totalMapPoints = 0;
int currentScanIndex = 0;
// Adress on our singleton
NRF24& radio = NRF24::getInstance();
extern "C" void CppMain(void) {

	// Channel_2 and 3 its our pins for servos. I using RC 360 servos 996
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);


    //Sending msg to stm32 console (we can hear this from our Computer by USB connection
    char startMsg[] = "\r\n STM32 wiezyczka zaczela dzialac \r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)startMsg, strlen(startMsg), 100);


    //Our canal of radio waves
    uint8_t AdresOdbiorczy[5] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE1};
    uint8_t AdresNadawczy[5]  = {0xE8, 0xE8, 0xF0, 0xF0, 0xE2};

    radio.Init();
    //radio.EnableAutoAck();
    radio.RxMode(AdresOdbiorczy, 115);

    //radio.EnableAckPayload();


    STMPacket startowaPaczka = {1500, 1500, 0, 0, 0, 0};
    radio.WriteAckPayload(0, (uint8_t*)&startowaPaczka, sizeof(startowaPaczka));

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1600);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1600);
    HAL_Delay(1000);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1500);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1500);



    uint32_t lastHeartbeat = HAL_GetTick();
    uint32_t ostatniSkan = HAL_GetTick();
    int16_t pulseXChanged=0;
    int16_t pulseYChanged=0;
    float currentPosX = 1500.0f;
    float currentPosY = 1500.0f;
    int16_t kierunekLidar = 1;
    DataPacket odebraneDane = {1500, 1500, 0, 0, 0, 0};
    uint8_t aktualnyTrybRadia = 255;
    int16_t scanX = 500;
    int16_t scanY = 900;
    int16_t scanDir = 50;
    bool skanowanieZakonczone = false;

    Laser laser(GPIOA, GPIO_PIN_8);
    Husky camera(&hi2c1);
    if(camera.ping()) {
        char msg[] = "Kamera HuskyLens gotowa!\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    }
    Lidar skaner;
    uint32_t ostatniRuchSkanera = 0;
    const uint32_t KROK_SKANERA_MS = 150;
    skaner.init();
    while (1) {
    	uint8_t rxCmd = 0;
//    	        if (HAL_UART_Receive(&huart2, &rxCmd, 1, 0) == HAL_OK) {
//    	            if (rxCmd == 'E' || rxCmd == 'e') {
//    	                char dumpStart[] = "Wysylka\n";
//    	                HAL_UART_Transmit(&huart2, (uint8_t*)dumpStart, strlen(dumpStart), 100);
//
//    	                for (int i = 0; i < totalMapPoints; i++) {
//    	                    char buf[64];
//    	                    sprintf(buf, "%d,%d,%u\r\n", lidarMap[i].x, lidarMap[i].y, lidarMap[i].distance);
//    	                    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);
//    	                }
//
//    	                char dumpEnd[] = "Koniec wysylki\n";
//    	                HAL_UART_Transmit(&huart2, (uint8_t*)dumpEnd, strlen(dumpEnd), 100);
//    	            }
//    	        }
    	if (HAL_GetTick() - lastHeartbeat > 2000) {
    	            char diagMsg[] = "STM32 nasluchuje...\r\n";
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
    	if (odebraneDane.modeStm == 1 || odebraneDane.modeStm == 4 || odebraneDane.modeStm == 7 ||
                odebraneDane.modeStm == 2 || odebraneDane.modeStm == 5 || odebraneDane.modeStm == 8 ||
                odebraneDane.modeStm == 12) {

    		if (odebraneDane.modeStm == 1 || odebraneDane.modeStm == 4 || odebraneDane.modeStm == 7) {
    		                HAL_I2C_IsDeviceReady(&hi2c1, 0x64, 3, 100);
    		                TrackingObj cel = camera.getTrackedObject();
    		                if (cel.isDet) {
    		                    int martwa_strefa = 20;
    		                    int krok_serwa = 15;

    		                    if (cel.xCenter > (160 + martwa_strefa)) currentPosX -= krok_serwa;
    		                    else if (cel.xCenter < (160 - martwa_strefa)) currentPosX += krok_serwa;

    		                    if (cel.yCenter > (120 + martwa_strefa)) currentPosY -= krok_serwa;
    		                    else if (cel.yCenter < (120 - martwa_strefa)) currentPosY += krok_serwa;

    		                    if (currentPosX < 500) currentPosX = 500;
    		                    if (currentPosX > 2500) currentPosX = 2500;
    		                    if (currentPosY < 1000) currentPosY = 1000;
    		                    if (currentPosY > 2000) currentPosY = 2000;

    		                    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (uint16_t)currentPosX);
    		                    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint16_t)currentPosY);
    		                    laser.shoot();
    		                }
    		            }
    		else if (odebraneDane.modeStm == 2 || odebraneDane.modeStm == 5 || odebraneDane.modeStm == 8) {
    		                if (!skanowanieZakonczone) {
    		                	if (HAL_GetTick() - ostatniRuchSkanera >= KROK_SKANERA_MS) {
    		                		ostatniRuchSkanera = HAL_GetTick();

    		                    currentPosX = scanX;
    		                    currentPosY = scanY;
    		                    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (uint16_t)currentPosX);
    		                    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint16_t)currentPosY);
    		                    uint16_t zmierzonyDystans = skaner.getDistance();
    		                    HAL_Delay(100);
    		                    char debugMsg[64];
    		                    sprintf(debugMsg, "X:%d Y:%d D:%umm\r\n", (int)currentPosX, (int)currentPosY, zmierzonyDystans);
    		                    HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), 100);

    		                    if (totalMapPoints < MAX_MAP_SIZE && zmierzonyDystans != 8888) {
    		                                lidarMap[totalMapPoints].x = (int16_t)currentPosX;
    		                                lidarMap[totalMapPoints].y = (int16_t)currentPosY;
    		                                lidarMap[totalMapPoints].distance = zmierzonyDystans;
    		                                totalMapPoints++;

    		                            }

    		                    scanX += scanDir;
    		                    if (scanX > 2400) {
    		                        scanX = 2400;
    		                        scanDir = -80;
    		                        scanY += 80;
    		                    } else if (scanX < 500) {
    		                        scanX = 500;
    		                        scanDir = 80;
    		                        scanY += 80;
    		                    }

    		                    if (scanY > 1600) {
    		                        skanowanieZakonczone = true;
    		                        char msgDone[] = "Skanowanie ukoczone\n";
    		                        HAL_UART_Transmit(&huart2, (uint8_t*)msgDone, strlen(msgDone), 100);
    		                    }
    		                	}
    		                }
    		            }
    		else if (odebraneDane.modeStm == 12) {
    		                if (totalMapPoints > 0) {


    		                	if (HAL_GetTick() - ostatniRuchSkanera >= KROK_SKANERA_MS) {
    		                	                        ostatniRuchSkanera = HAL_GetTick();
    		                	MapPoint punkt = lidarMap[currentScanIndex];

    		                    currentPosX = punkt.x;
    		                    currentPosY = punkt.y;
    		                    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (uint16_t)currentPosX);
    		                    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint16_t)currentPosY);

    		                    HAL_Delay(30);
    		                    uint16_t aktualnyDystans = skaner.getDistance();
    		                    int roznica = abs((int)aktualnyDystans - (int)punkt.distance);
    		                    char debugMsg[64];
    		                    sprintf(debugMsg, "X:%d Y:%d D:%umm\r\n", (int)currentPosX, (int)currentPosY, roznica);
    		                    HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), 100);

    		                    if (roznica > 1200) {
    		                        laser.shoot();
    		                    }
    		                    currentScanIndex++;
    		                    if (currentScanIndex >= totalMapPoints) {
    		                        currentScanIndex = 0;
    		                    }
    		                	}
    		                }
    		            }

    		STMPacket doWyslania = {222, (int16_t)currentPosY, 0, 0, 0, 0, 0};    		                    doWyslania.stmX = (int16_t)currentPosX;
    		//radio.WriteAckPayload(0, (uint8_t*)&doWyslania, sizeof(doWyslania));
    		if (aktualnyTrybRadia != 1) {
    		                radio.TxMode(AdresNadawczy, 115);
    		                aktualnyTrybRadia = 1;
    		            }
    		                    // Wysyłamy mapę w eter

    		                    radio.Transmit((uint8_t*)&doWyslania, sizeof(doWyslania));

    		                    // Odczekujemy chwilę (zastępuje to ping-ponga, stabilizuje pętlę)
    		                    HAL_Delay(50);

    		                    // Co 4 sekundy sprawdzamy, czy użytkownik nie uderzył w klawiaturę
    		                    if (HAL_GetTick() - ostatniSkan > 4000) {
    		                        radio.RxMode(AdresOdbiorczy, 115);
    		                        aktualnyTrybRadia = 0;
    		                        uint32_t startNasluchu = HAL_GetTick();

    		                        // Otwieramy okno na 500 ms na wyłapanie komendy zmiany trybu z ESP32
    		                        while(HAL_GetTick() - startNasluchu < 500) {
    		                            if(radio.IsDataAvailable()) {
    		                                DataPacket nowaPaczka;
    		                                radio.Receive((int*)&nowaPaczka);

    		                                const int16_t Klucz = 24897;
    		                                const int16_t Klucz3 = 13579; // Nowy klucz szyfrowania

    		                                // Dekodowanie z uwzględnieniem Szyfru 3 oraz LiDARa
    		                                if (nowaPaczka.modeStm >= 3 && nowaPaczka.modeStm <= 5) {
    		                                    nowaPaczka.speedX += 952; nowaPaczka.speedY += 952; nowaPaczka.modeStm -= 3;
    		                                } else if (nowaPaczka.modeStm >= 6 && nowaPaczka.modeStm <= 8) {
    		                                    nowaPaczka.speedX ^= Klucz; nowaPaczka.speedY ^= Klucz; nowaPaczka.modeStm -= 6;
    		                                }

    		                                // Zapisujemy paczkę - pozwala nam to swobodnie przeskakiwać np. z Auto do LiDARa

    		                                // Uciekamy z bloku nadawania tylko jeśli zmieniamy na czysty WSAD
    		                                if (nowaPaczka.modeStm == 0 || nowaPaczka.modeStm == 3 || nowaPaczka.modeStm == 6) {    		                                    char msgStop[] = "Przejscie do WSAD!\r\n";
    		                                odebraneDane = nowaPaczka;
    		                                skanowanieZakonczone = false;
    		                                scanX = 500;
    		                                scanY = 900;
    		                                scanDir = 50;
    		                                totalMapPoints = 0;
    		                                break;
    		                                }
    		                                else {
    		                                    odebraneDane = nowaPaczka;
    		                                }
    		                            }
    		                        }
    		                        ostatniSkan = HAL_GetTick();
    		                    }
    		                }

    		                else {
    		                    // Reset nasłuchu sprzętowego
    		                    if (aktualnyTrybRadia != 0) {
    		                        radio.RxMode(AdresOdbiorczy, 115);
    		                        aktualnyTrybRadia = 0;
    		                    }

    		                    if (radio.IsDataAvailable()) {
    		                        radio.Receive((int*)&odebraneDane);

    		                        const int16_t Klucz = 24897;
    		                        //const int16_t Klucz3 = 13579;

    		                        if (odebraneDane.modeStm >= 3 && odebraneDane.modeStm <= 5) {
    		                                            odebraneDane.speedX += 952; odebraneDane.speedY += 952;
    		                                        } else if (odebraneDane.modeStm >= 6 && odebraneDane.modeStm <= 8) {
    		                                            odebraneDane.speedX ^= Klucz; odebraneDane.speedY ^= Klucz;
    		                                        }

    		                        laser.update();

    		                        // WSAD
    		                        if (odebraneDane.modeStm == 0 || odebraneDane.modeStm == 3 || odebraneDane.modeStm == 6) {    		                            char sizeMsg2[50];
    		                            sprintf(sizeMsg2, "Tryb WSAD\r\n");
    		                            // HAL_UART_Transmit(&huart2, (uint8_t*)sizeMsg2, strlen(sizeMsg2), 100);

    		                            currentPosX = odebraneDane.speedX;
    		                            currentPosY = odebraneDane.speedY;

    		                            char wiadomosc[64];
    		                            sprintf(wiadomosc, "Odebrano PWM X: %d, Y: %d \n", (int)currentPosX, (int)currentPosY);
    		                            HAL_UART_Transmit(&huart2, (uint8_t*)wiadomosc, strlen(wiadomosc), 100);

    		                            if (currentPosX < 500) currentPosX = 500;
    		                            if (currentPosX > 2500) currentPosX = 2500;
    		                            if (currentPosY < 1000) currentPosY = 1000;
    		                            if (currentPosY > 2000) currentPosY = 2000;

    		                            if (odebraneDane.laserShot == 1) laser.shoot();

    		                            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint16_t)currentPosY);
    		                            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (uint16_t)currentPosX);
    		                        }

    		//                      if(status == HAL_OK) {
    		//                          laser.shoot();
    		//                      }
    		                    }
    		                }
    		            }
    		}
