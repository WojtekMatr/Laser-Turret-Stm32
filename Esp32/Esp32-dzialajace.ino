#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//RF24 radio(4, 5); 
//const byte address[5] = {0x11, 0x11, 0x11, 0x11, 0x11};

class Radio24GHz{
RF24 radio;
//FHSS (Frequency Hopping Spread Spectrum) Jest to standard wojskowy
// jako ze w przestrzeni fal radiowych jammery nie blokowaly komunikacji miedzy 
// urzadzeniami
const byte adressSTM[5] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE1};
const byte adressESP[5] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE2};
const byte tablicaAdresow[4][5] = {
    {0xE8, 0xE8, 0xF0, 0xF0, 0xE1},
    {0xAB, 0xCD, 0xEF, 0x12, 0x01}, 
    {0x55, 0x66, 0x77, 0x88, 0x02}, 
    {0x12, 0x34, 0x56, 0x78, 0x03}  
};


public:
Radio24GHz() : radio(4,5){}
 RF24& getRadio(){
  return radio;
}
    
const byte* getAddressSTM() {
        return adressSTM;
    }
const byte* getAddressESP() {
        return adressESP;
    }
};
Radio24GHz radio24 = Radio24GHz();


struct __attribute__((packed)) DataPacket { //Paczka do nadawania do STM
    int16_t speedX;
    int16_t speedY;
    int16_t laserOnOff;
    int16_t laserShot;
    int16_t modeStm;
    int16_t modeServo; //odpowiada za Adresy 
};

struct __attribute__((packed)) STMPacket { //Paczka do obierania od STM
    int16_t stmX;        // Fizyczna pozycja serwa X
    int16_t stmY;        // Fizyczna pozycja serwa Y
    int16_t huskyX;          // wspolrzedna X z HuskyLens 
    int16_t huskyY;          // wspolrzedna Y z HuskyLens 
    uint8_t obiektWykryty;   // Czy widzimy cel
    uint8_t laserStrzelil;   
    int16_t padding;
};
STMPacket paczkaOdebrana;

int16_t szyfrujDane = 0;
const int16_t Klucz = 24897;
uint8_t aktualnyAdresRadia = 0;
bool autoZmienianie = false;
unsigned long ostatniSkokAdresu = 0;
DataPacket paczka;

void setup() {
  Serial.begin(115200);

  if (!radio24.getRadio().begin()) {
    while (1); 
  }
  
  radio24.getRadio().setChannel(115);
  radio24.getRadio().setDataRate(RF24_1MBPS);
  radio24.getRadio().setCRCLength(RF24_CRC_16); 
  //radio24.getRadio().setAutoAck(false);
  //radio24.getRadio().setRetries(5, 15);
  //radio24.getRadio().setPayloadSize(12); // 8 bajty
  radio24.getRadio().setAutoAck(false);
  radio24.getRadio().disableDynamicPayloads();
  //radio24.getRadio().enableDynamicPayloads();  
  radio24.getRadio().openWritingPipe(radio24.getAddressSTM());
  radio24.getRadio().openReadingPipe(1, radio24.getAddressESP());
  radio24.getRadio().setPayloadSize(12);
  radio24.getRadio().setPALevel(RF24_PA_MAX); 
  radio24.getRadio().stopListening();


//  radio24.getRadio().enableAckPayload();       
 // radio24.getRadio().enableDynamicPayloads();
 // radio24.getRadio().setDataRate(RF24_1MBPS);
  //radio24.getRadio().setCRCLength(RF24_CRC_16); 
 // radio24.getRadio().openWritingPipe(radio24.getAddress());
  //radio24.getRadio().setPALevel(RF24_PA_LOW);   
  
  // Startujemy od zera (serwa stoją w miejscu na impulsie 1500 w STM32)
paczka.speedX = 1500;
paczka.speedY = 1500;
paczka.laserOnOff = 0;
paczka.laserShot = 0;
paczka.modeStm = 0;  
paczka.modeServo = 0;
  
  Serial.println("ESP32 gotowe");
if (radio24.getRadio().isChipConnected()) {
    Serial.println("nRF24 jest poprawnie podlaczony");
} else {
    Serial.println("Brak komunikacji miedzy ESP a nRF24");
}
}

void loop() {
  bool czyKliknelismy = false; // zmienna by wyswietlac w serial monitorze,
  // jesli zmienimy cos w pozycji zaszyfrowane
  
  // czy cos jest z klawiatury
  if (Serial.available() > 0) {
    czyKliknelismy = true;
    char znak = Serial.read();
    
    // niewidoczne znaki ignorowane
        if (znak != '\n' && znak != '\r') {
               
        // Zmiana trybów pracy (modeStm)
        if (znak == '0') {
            paczka.modeStm = 0;
            szyfrujDane = 0;
            Serial.println("WSAD");
        }
        else if (znak == '1') {
            paczka.modeStm = 1;
            szyfrujDane = 0;
            Serial.println("Auto");
        }
        else if (znak == '2') {
            paczka.modeStm = 2;
            szyfrujDane = 0;
            Serial.println("Skanowanie");
        }
        else if (znak == '3') {
            paczka.modeStm = 3;
            szyfrujDane = 1;
            Serial.println("WSAD - szyfrowanie cezar");
        }
        else if (znak == '4') {
            paczka.modeStm = 4;
            szyfrujDane = 1;
            Serial.println("Auto - szyfrowanie cezar");
        }
        else if (znak == '5') {
            paczka.modeStm = 5;
            szyfrujDane = 1;
            Serial.println("Skanowanie - szyfrowanie cezar");
        }

        else if (znak == '6') {
            paczka.modeStm = 6;
            szyfrujDane = 2;
            Serial.println("WSAD - szyfrowanie kluczem");
        }
        else if (znak == '7') {
            paczka.modeStm = 7;
            szyfrujDane = 2;
            Serial.println("Auto - szyfrowanie kluczem");
        }
        else if (znak == '8') {
            paczka.modeStm = 8;
            szyfrujDane = 2;
            Serial.println("Skanowanie - szyfrowanie cezar");
        }
        else if (znak == '9') {
             paczka.modeStm = 12; 
        szyfrujDane = 0; 
        Serial.println("Auto-LiDAR"); }

        // Sterowanie 
        else if (znak == 'w' || znak == 'W') paczka.speedY += 50;
        else if (znak == 's' || znak == 'S') paczka.speedY -= 50;
        else if (znak == 'd' || znak == 'D') paczka.speedX += 50;
        else if (znak == 'a' || znak == 'A') paczka.speedX -= 50;
        
        // Laser 
        else if (znak == 'f' || znak == 'F') {
            paczka.laserShot = 1;
        }

        // zerowanie
        else if (znak == ' ') {
            paczka.speedX = 1500;
            paczka.speedY = 1500;
        }
        else if (znak == 'z' || znak == 'Z') {
            autoZmienianie = false; 
            paczka.modeServo = 0;
        }
        else if (znak == 'x' || znak == 'X') {
            autoZmienianie = false;
            paczka.modeServo = 1;
        }
        else if (znak == 'c' || znak == 'C') {
            autoZmienianie = false;
            paczka.modeServo = 2;
        }
        else if (znak == 'v' || znak == 'V') {
            autoZmienianie = false;
            paczka.modeServo = 3;
        }
        
        else if (znak == 'b' || znak == 'B') {
            autoZmienianie = !autoZmienianie;
            if (autoZmienianie) {
                ostatniSkokAdresu = millis();
            }
        }

        // blokawy
        if (paczka.speedX > 2600) paczka.speedX = 2600;
        if (paczka.speedX < 500) paczka.speedX = 500;
        if (paczka.speedY > 2000) paczka.speedY = 2000;
        if (paczka.speedY < 900) paczka.speedY = 900;
        
        // 


    }
  }
  DataPacket paczkaDoWysylki = paczka; //paczka1 jest do wyslania, poniewaz paczka musi operowac +50/ -50
  // bez tego by zaszyfrowana sie psula
  if (szyfrujDane == 1) {
      paczkaDoWysylki.speedX -= 952; 
      paczkaDoWysylki.speedY -= 952;
  }
  if (szyfrujDane == 2) {
  paczkaDoWysylki.speedX = paczkaDoWysylki.speedX ^ Klucz;
    paczkaDoWysylki.speedY = paczkaDoWysylki.speedY ^ Klucz;
  }
if (paczka.modeStm == 1 || paczka.modeStm == 4 || paczka.modeStm == 7 ||
      paczka.modeStm == 2 || paczka.modeStm == 5 || paczka.modeStm == 8 || 
      paczka.modeStm == 12) {
      radio24.getRadio().startListening();
      
      unsigned long startNasluchu = millis();
            while (millis() - startNasluchu < 200) { 
          if (radio24.getRadio().available()) {
              radio24.getRadio().read(&paczkaOdebrana, sizeof(paczkaOdebrana));
              Serial.print("MAPA -> X:");
              Serial.print(paczkaOdebrana.stmX);
              Serial.print(" | Y:");
              Serial.println(paczkaOdebrana.stmY);
              
          }
      }

      radio24.getRadio().stopListening();
      delay(2);
      radio24.getRadio().write(&paczkaDoWysylki, sizeof(paczkaDoWysylki));
  } 
  else {
      radio24.getRadio().stopListening();
      radio24.getRadio().write(&paczkaDoWysylki, sizeof(paczkaDoWysylki));
      delay(30); 
  }
  paczka.laserShot = 0;
}