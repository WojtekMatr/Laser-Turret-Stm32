#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(4, 5); 
const byte address[5] = {0x11, 0x11, 0x11, 0x11, 0x11};

// Zamiast kątów, trzymamy bezpośrednio wartości impulsów PWM (1000 - 2000).
// Wartość 1500 to punkt zerowy (STOP) dla serw 360 stopni.
int16_t pwmX = 1500; 
int16_t pwmY = 1500;

// Tablica przechowująca oba sygnały (2 x 2 bajty = 4 bajty paczki)
int16_t payload[2];

void setup() {
  Serial.begin(115200);

  if (!radio.begin()) {
    Serial.println("Blad: Nie wykryto modulu nRF24 na ESP32!");
    while (1); 
  }
  
  radio.setChannel(10);
  radio.setAutoAck(false);
  radio.setPayloadSize(sizeof(payload)); 
  radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_16); 
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);       
  radio.stopListening();
  
  Serial.println("--- Baza ESP32 gotowa (SERWA 360) ---");
  Serial.println("W/S/A/D - Plynna zmiana predkosci | X - Nagly STOP");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    cmd = toupper(cmd); // Zamiana na wielką literę

    bool positionChanged = false;

    // Sterowanie osią Y (W/S) - Zmiana o 50 mikrosekund (regulacja prędkości)
    if (cmd == 'W') {
      pwmY += 50; 
      positionChanged = true;
    } else if (cmd == 'S') {
      pwmY -= 50; 
      positionChanged = true;
    } 
    // Sterowanie osią X (A/D)
    else if (cmd == 'A') {
      pwmX -= 50;
      positionChanged = true;
    } else if (cmd == 'D') {
      pwmX += 50;
      positionChanged = true;
    }
    // HAMULEC (X) - Natychmiastowe zatrzymanie obu serw
    else if (cmd == 'X') {
      pwmX = 1500;
      pwmY = 1500;
      positionChanged = true;
    }

    if (positionChanged) {
      // Pilnujemy sprzętowego zakresu sygnału PWM (1000 - 2000)
      pwmX = constrain(pwmX, 1000, 2000);
      pwmY = constrain(pwmY, 1000, 2000);

      payload[0] = pwmX;
      payload[1] = pwmY;

      radio.write(&payload, sizeof(payload));
      
      Serial.print("Wyslano PWM -> X: ");
      Serial.print(pwmX);
      Serial.print(" | Y: ");
      Serial.println(pwmY);
    }
  }
}
