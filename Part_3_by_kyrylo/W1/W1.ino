#include <SPI.h>
#include <LoRa.h>

uint8_t lastRands[10];
uint8_t randCount = 0;
uint8_t randIndex = 0;

void initLoRa() {
  Serial.begin(9600);
  while (!Serial);
  if (!LoRa.begin(868E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  Serial.println("=== LoRa ready ===");
}

bool processRand(uint8_t Rand) {
  Serial.print("Last Rands: ");
  for (uint8_t i = 0; i < randCount; i++) {
    Serial.print("0b");
    Serial.print(lastRands[i], BIN);
    Serial.print(" ");
  }
  if (randCount == 0) Serial.print("<empty>");
  Serial.println();

  bool duplicate = false;
  for (uint8_t i = 0; i < randCount; i++) {
    if (lastRands[i] == Rand) {
      duplicate = true;
      break;
    }
  }
  if (duplicate) {
    Serial.print("⚠️ Duplicate Rand detected: 0b");
    Serial.println(Rand, BIN);
  } else {
    Serial.print("✅ No duplicate for Rand: 0b");
    Serial.println(Rand, BIN);
  }

  lastRands[randIndex] = Rand;
  randIndex = (randIndex + 1) % 10;
  if (randCount < 10) randCount++;

  return duplicate;
}


void modifyAndResend(uint8_t packet[7], uint8_t fieldIndex, uint8_t newValue) {
  if (fieldIndex < 1 || fieldIndex > 6) return;
  newValue &= 0x0F;

  uint8_t byteIdx = 2 + (fieldIndex - 1) / 2;
  bool highNibble = (fieldIndex % 2 == 1);

  if (highNibble) {
    packet[byteIdx] = (newValue << 4) | (packet[byteIdx] & 0x0F);
  } else {
    packet[byteIdx] = (packet[byteIdx] & 0xF0) | newValue;
  }

  LoRa.beginPacket();
    LoRa.write(packet, 7);
  LoRa.endPacket();
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

  for (uint8_t i = 0; i < 7; i++) {
    Serial.print("[");
    if (packet[i] < 0x10) Serial.print('0');
    Serial.print(packet[i], HEX);
    Serial.print("]");
  }
  Serial.println();

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

  bool duplicate = processRand(IDpaket);

  if (!duplicate) {
    modifyAndResend(packet, 1, 0b0001);
    Serial.println("Resent modified packet (W1 set to 0b0001).");
  }
}

void setup() {
  initLoRa();
  randomSeed(analogRead(A0));
}

void loop() {
  receiveMessage();
}
