#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//RF24 radio(4, 5); 
//const byte address[5] = {0x11, 0x11, 0x11, 0x11, 0x11};

class Radio24GHz{
RF24 radio;
const byte address[5] = {0x11, 0x11, 0x11, 0x11, 0x11};
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

struct DataPacket {
  int16_t speedX;
  int16_t speedY;
};


DataPacket paczka;

void setup() {
  Serial.begin(115200);

  if (!radio24.getRadio().begin()) {
    Serial.println("Blad: Nie wykryto modulu nRF24 na ESP32!");
    while (1); 
  }
  
  radio24.getRadio().setChannel(10);
  radio24.getRadio().setAutoAck(false);
  radio24.getRadio().setPayloadSize(4); // 4 bajty
  radio24.getRadio().setDataRate(RF24_1MBPS);
  radio24.getRadio().setCRCLength(RF24_CRC_16); 
  radio24.getRadio().openWritingPipe(radio24.getAddress());
  radio24.getRadio().setPALevel(RF24_PA_MIN);       
  radio24.getRadio().stopListening();
  
  // Startujemy od zera (serwa stoją w miejscu na impulsie 1500 w STM32)
  paczka.speedX = 0;
  paczka.speedY = 0;
  
  Serial.println("--- Baza ESP32 gotowa (TRYB KLAWIATURY WSAD) ---");
  Serial.println("Wpisz W/S zeby sterowac Y");
  Serial.println("Wpisz A/D zeby sterowac X");
  Serial.println("Wpisz SPACJE zeby zatrzymac serwa (0, 0)");
}

void loop() {
  // Sprawdzamy, czy przyszło coś z klawiatury w Serial Monitorze
  if (Serial.available() > 0) {
    char znak = Serial.read();
    
    // Ignorujemy niewidoczne znaki nowej linii
    if (znak == '\n' || znak == '\r') return;

    // Sterowanie prędkością (wielkość liter nie ma znaczenia)
    if (znak == 'w' || znak == 'W') paczka.speedY += 15;
    else if (znak == 's' || znak == 'S') paczka.speedY -= 15;
    else if (znak == 'd' || znak == 'D') paczka.speedX += 15;
    else if (znak == 'a' || znak == 'A') paczka.speedX -= 15;
    else if (znak == ' ') {
      // Bezpieczny hamulec
      paczka.speedX = 0;
      paczka.speedY = 0;
    }

    // Zabezpieczenie przed wyjściem poza nasz założony zakres (-100 do 100)
    if (paczka.speedX > 100) paczka.speedX = 100;
    if (paczka.speedX < -100) paczka.speedX = -100;
    if (paczka.speedY > 100) paczka.speedY = 100;
    if (paczka.speedY < -100) paczka.speedY = -100;
    
    // Wypisujemy tylko wtedy, gdy wciskamy klawisz (żeby nie zaspamować konsoli)
    Serial.print("Nowa predkosc -> X: ");
    Serial.print(paczka.speedX);
    Serial.print(" | Y: ");
    Serial.println(paczka.speedY);
  }

  // Wysyłamy paczkę do STM32 ciągle, żeby wieżyczka nie straciła zasięgu
  radio24.getRadio().write(&paczka, sizeof(paczka));
  
  delay(50); // Krótkie opóźnienie, pętla leci bardzo szybko
}