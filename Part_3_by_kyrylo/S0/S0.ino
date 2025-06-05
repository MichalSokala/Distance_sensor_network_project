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
  Serial.print(Data);

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

  if (src == 0b1001
      && IDW1 == 0
      && IDW2 == 0
      && IDW3 == 0
      && IDW4 == 0
      && IDW5 == 0
      && IDW6 == 0) {
    return;
  }
  checkSixOneShot(IDW1, IDW2, IDW3,
                   IDW4, IDW5, IDW6);

  printFrameBinary(IDpaket, src, dest,
                   IDW1, IDW2, IDW3,
                   IDW4, IDW5, IDW6,
                   Data);
}


void checkSixOneShot(uint8_t v0, uint8_t v1, uint8_t v2,
                     uint8_t v3, uint8_t v4, uint8_t v5)
{
  uint8_t values[6] = { v0, v1, v2, v3, v4, v5 };
  static bool flags[6] = { false, false, false, false, false, false };
  static unsigned long startTime[6] = { 0, 0, 0, 0, 0, 0 };

  unsigned long now = millis();

  for (uint8_t i = 0; i < 6; i++) {
    if (values[i] != 0x00 && !flags[i]) {
      flags[i] = true;
      startTime[i] = now; 
      Serial.print("["); Serial.print(now); Serial.print("] ");
      Serial.print("v"); Serial.print(i);
      Serial.println(" became true (start 10s timer)");
    }

    if (flags[i] && (now - startTime[i] >= 10000UL)) {
      flags[i] = false;
      Serial.print("["); Serial.print(now); Serial.print("] ");
      Serial.print("v"); Serial.print(i);
      Serial.println(" reset to false (10s elapsed)");
    }

    Serial.print("["); Serial.print(now); Serial.print("] ");
    Serial.print("v"); Serial.print(i); Serial.print(" ");
    Serial.println(flags[i] ? "true" : "false");
  }

  Serial.println();
}

void setup() {
  initLoRa();
  randomSeed(analogRead(A0));
}

void loop() {
  receiveMessage();
}
