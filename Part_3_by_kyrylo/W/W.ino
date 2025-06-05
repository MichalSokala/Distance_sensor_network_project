#include <SPI.h>
#include <LoRa.h>

uint8_t lastRands[10];
uint8_t randCount = 0;
uint8_t randIndex = 0;

unsigned long lastReceiveTime = 0;


uint8_t number = 1; // 2 3 4 5 6
uint8_t W      = 0b0001; // 0b0010 0b0011 0b0100 0b0101 0b0110 


// Funkcja wysyłająca wiadomość „ZYJE” jako heartbeat
void sendAlive() {
  uint8_t  myIDpaket = generateRandomByte();
  uint8_t  mySrc     = W;
  uint8_t  myDest    = 0b0000;

  uint8_t  myIDW1    = 0b0000;
  uint8_t  myIDW2    = 0b0000;
  uint8_t  myIDW3    = 0b0000;
  uint8_t  myIDW4    = 0b0000;
  uint8_t  myIDW5    = 0b0000;
  uint8_t  myIDW6    = 0b0000;

  uint16_t myData    = 0b0000000000000000;


  sendMessage(myIDpaket, mySrc, myDest,
              myIDW1, myIDW2, myIDW3,
              myIDW4, myIDW5, myIDW6,
              myData);

}

void BlinkLED(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
    delay(300);                      // Wait 300 ms
    digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
    delay(300);                      // Wait 300 ms before the next blink
  }
}

void initLoRa() {
  Serial.begin(9600);

  if (!LoRa.begin(868E6)) {
    // Serial.println("LoRa init failed!");
    while (1);
  }
  // Serial.println("=== LoRa ready ===");
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
  BlinkLED(2);
}

bool processRand(uint8_t Rand) {
  // Serial.print("Last Rands: ");
  for (uint8_t i = 0; i < randCount; i++) {
    // Serial.print("0b");
    // Serial.print(lastRands[i], BIN);
    // Serial.print(" ");
  }
  // if (randCount == 0) Serial.print("<empty>");
  // Serial.println();

  bool duplicate = false;
  for (uint8_t i = 0; i < randCount; i++) {
    if (lastRands[i] == Rand) {
      duplicate = true;
      break;
    }
  }
  if (duplicate) {
    // Serial.print("⚠️ Duplicate Rand detected: 0b");
    // Serial.println(Rand, BIN);
  } else {
    // Serial.print("✅ No duplicate for Rand: 0b");
    // Serial.println(Rand, BIN);
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
    // Serial.print("[");
    // if (packet[i] < 0x10) Serial.print('0');
    // Serial.print(packet[i], HEX);
    // Serial.print("]");
  }
  // Serial.println();

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
    modifyAndResend(packet, number, W);
    // Serial.println("Resent modified packet.");
  }
}

void setup() {
  initLoRa();
  randomSeed(analogRead(A0));
  pinMode(LED_BUILTIN, OUTPUT);
  // Na starcie ustawiamy lastReceiveTime na teraz,
  // żeby po włączeniu nie wysyłać od razu ŻYJĘ
  lastReceiveTime = millis();
}

void loop() {
  receiveMessage();

  // Sprawdzamy, czy minęło ponad 15 sekund od ostatniego odbioru
  if (millis() - lastReceiveTime > 15000UL) {
    // Jeśli tak, wysyłamy wiadomość ŻYJĘ i resetujemy timer
    sendAlive();
    lastReceiveTime = millis();
  }

}
