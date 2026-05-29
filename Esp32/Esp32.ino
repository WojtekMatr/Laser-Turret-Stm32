#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//RF24 radio(4, 5); 
//const byte address[5] = {0x11, 0x11, 0x11, 0x11, 0x11};

class Radio24GHz{
RF24 radio;
const byte address[5] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE1};
public:
Radio24GHz() : radio(4,5){}
 RF24& getRadio(){
  return radio;
}
const byte* getAddress() {
        return address;
    }
};
Radio24GHz radio24 = Radio24GHz();

// STRUKTURA IDENTYCZNA JAK NA STM32

struct __attribute__((packed)) DataPacket {
    int16_t speedX;
    int16_t speedY;
    int16_t laserOnOff;
    int16_t laserShot;
    int16_t modeStm;
    int16_t modeServo;
};


DataPacket paczka;

void setup() {
  Serial.begin(115200);

  if (!radio24.getRadio().begin()) {
    while (1); 
  }
  
  radio24.getRadio().setChannel(115);
  radio24.getRadio().setAutoAck(false);
  radio24.getRadio().setPayloadSize(12); // 8 bajty
  radio24.getRadio().setDataRate(RF24_1MBPS);
  radio24.getRadio().setCRCLength(RF24_CRC_16); 
  radio24.getRadio().openWritingPipe(radio24.getAddress());
  radio24.getRadio().setPALevel(RF24_PA_LOW);   
  radio24.getRadio().stopListening();
  
  // Startujemy od zera (serwa stoją w miejscu na impulsie 1500 w STM32)
paczka.speedX = 1500;
paczka.speedY = 1500;
paczka.laserOnOff = 0;
paczka.laserShot = 0;
paczka.modeStm = 0;  
paczka.modeServo = 0;
  
  Serial.println("ESP32");

}

void loop() {
  // czy cos jest z klawiatury
  if (Serial.available() > 0) {
    char znak = Serial.read();
    
    // niewidoczne znaki ignorowane
        if (znak != '\n' && znak != '\r') {
        
        // Sterowanie 
        if (znak == 'w' || znak == 'W') paczka.speedY += 50;
        else if (znak == 's' || znak == 'S') paczka.speedY -= 50;
        else if (znak == 'd' || znak == 'D') paczka.speedX += 50;
        else if (znak == 'a' || znak == 'A') paczka.speedX -= 50;
        
        // Laser 
        else if (znak == 'f' || znak == 'F') {
            paczka.laserShot = 1;
        }
        // Zmiana trybów pracy (modeStm)
        else if (znak == '0') {
            paczka.modeStm = 0;
            Serial.println("WSAD");
        }
        else if (znak == '1') {
            paczka.modeStm = 1;
            Serial.println("Auto");
        }
        else if (znak == '2') {
            paczka.modeStm = 2;
            Serial.println("Skanowanie");
        }
        // zerowanie
        else if (znak == ' ') {
            paczka.speedX = 1500;
            paczka.speedY = 1500;
        }

        // blokawy
        if (paczka.speedX > 2500) paczka.speedX = 2500;
        if (paczka.speedX < 500) paczka.speedX = 500;
        if (paczka.speedY > 2000) paczka.speedY = 2000;
        if (paczka.speedY < 1000) paczka.speedY = 1000;
        
        // 
        Serial.print("pozycja:");
        Serial.print(paczka.speedX);
        Serial.print(",");
        Serial.println(paczka.speedY);

    }
  }
  bool wyslano = radio24.getRadio().write(&paczka, sizeof(paczka));

  if (!wyslano) {
      Serial.println("blad z STM32");
  }
  paczka.laserShot = 0;
  delay(50); // krotkie opoznienie by nie spalic procesora
}