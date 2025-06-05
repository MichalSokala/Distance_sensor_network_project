#include <SPI.h>
#include <LoRa.h>
#include <Ultrasonic.h>

Ultrasonic ultrasonic(2, 3);
int distance;

void initLoRa() {
  Serial.begin(9600);
  while (!Serial);
  if (!LoRa.begin(868E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  Serial.println("=== LoRa ready ===");
}


uint8_t generateRandomByte() {
  return (uint8_t)random(0, 256);
}


void sendMessage(uint8_t IDpaket,
                 uint8_t src,
                 uint8_t dest,
                 uint8_t IDW1,
                 uint8_t IDW2,
                 uint8_t IDW3,
                 uint8_t IDW4,
                 uint8_t IDW5,
                 uint8_t IDW6,
                 uint16_t Data){
  uint8_t packet[7];
  packet[0] = IDpaket;                             // 8 bit
  packet[1] = (src << 4)   | (dest & 0x0F);        // 4 bit src | 4 bit dest
  packet[2] = (IDW1 << 4)  | (IDW2 & 0x0F);        // 4 bit IDW1 | 4 bit IDW2
  packet[3] = (IDW3 << 4)  | (IDW4 & 0x0F);        // 4 bit IDW3 | 4 bit IDW4
  packet[4] = (IDW5 << 4)  | (IDW6 & 0x0F);        // 4 bit IDW5 | 4 bit IDW6
  packet[5] = (Data >> 8)  & 0xFF;                 // Data (16 bit)
  packet[6] =  Data        & 0xFF;                 // Data (16 bit)

  LoRa.beginPacket();
    LoRa.write(packet, sizeof(packet));
  LoRa.endPacket();
}

void setup() {
  initLoRa();
  randomSeed(analogRead(A0));
}

void loop() {
  uint8_t  myIDpaket = generateRandomByte();
  uint8_t  mySrc     = 0b1001;
  uint8_t  myDest    = 0b0000;

  uint8_t  myIDW1    = 0b0000;
  uint8_t  myIDW2    = 0b0000;
  uint8_t  myIDW3    = 0b0000;
  uint8_t  myIDW4    = 0b0000;
  uint8_t  myIDW5    = 0b0000;
  uint8_t  myIDW6    = 0b0000;

  distance = ultrasonic.read();
  
  // uint16_t myData    = 0b0000111100001111;
  uint16_t myData    = (uint16_t)distance;

  sendMessage(myIDpaket, mySrc, myDest,
              myIDW1, myIDW2, myIDW3,
              myIDW4, myIDW5, myIDW6,
              myData);
  Serial.println("Send");
  Serial.println(distance);
  delay(2000);
}
