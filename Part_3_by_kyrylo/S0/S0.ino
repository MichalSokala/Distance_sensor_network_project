#include <SPI.h>
#include <LoRa.h>

void initLoRa() {
  Serial.begin(9600);
  while (!Serial);
  if (!LoRa.begin(868E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  Serial.println("=== LoRa ready ===");
}

void printBinary(uint32_t value, uint8_t bits) {
  Serial.print("0b");
  for (int8_t i = bits - 1; i >= 0; i--) {
    Serial.print((value >> i) & 0x1);
  }
}

void printFrameBinary(uint8_t IDpaket,
                      uint8_t src,
                      uint8_t dest,
                      uint8_t IDW1,
                      uint8_t IDW2,
                      uint8_t IDW3,
                      uint8_t IDW4,
                      uint8_t IDW5,
                      uint8_t IDW6,
                      uint16_t Data)
{
  Serial.print("Recv frame:\n  IDpaket = ");
  printBinary(IDpaket, 8);

  Serial.print("\n  src     = ");
  printBinary(src, 4);

  Serial.print("\n  dest    = ");
  printBinary(dest, 4);

  Serial.print("\n  IDW1    = ");
  printBinary(IDW1, 4);

  Serial.print("\n  IDW2    = ");
  printBinary(IDW2, 4);

  Serial.print("\n  IDW3    = ");
  printBinary(IDW3, 4);

  Serial.print("\n  IDW4    = ");
  printBinary(IDW4, 4);

  Serial.print("\n  IDW5    = ");
  printBinary(IDW5, 4);

  Serial.print("\n  IDW6    = ");
  printBinary(IDW6, 4);

  Serial.print("\n  Data    = ");
  printBinary(Data, 16);

  Serial.println("\n");
}

void receiveMessage() {
  int packetSize = LoRa.parsePacket();
  if (packetSize <= 0) return;

  if (packetSize != 7) {
    while (LoRa.available()) {
      LoRa.read();
    }
    return;
  }

  uint8_t packet[7];
  LoRa.readBytes(packet, 7);

  uint8_t IDpaket = packet[0];

  uint8_t src   = (packet[1] >> 4) & 0x0F;
  uint8_t dest  = packet[1] & 0x0F;

  uint8_t IDW1  = (packet[2] >> 4) & 0x0F;
  uint8_t IDW2  = packet[2] & 0x0F;

  uint8_t IDW3  = (packet[3] >> 4) & 0x0F;
  uint8_t IDW4  = packet[3] & 0x0F;

  uint8_t IDW5  = (packet[4] >> 4) & 0x0F;
  uint8_t IDW6  = packet[4] & 0x0F;

  uint16_t Data = ((uint16_t)packet[5] << 8) | packet[6];

  if (IDW1 == 0 && IDW2 == 0 && IDW3 == 0 &&
      IDW4 == 0 && IDW5 == 0 && IDW6 == 0) {
    return;
  }

  printFrameBinary(IDpaket, src, dest,
                   IDW1, IDW2, IDW3,
                   IDW4, IDW5, IDW6,
                   Data);
}

void setup() {
  initLoRa();
  randomSeed(analogRead(A0));
}

void loop() {
  receiveMessage();
}
