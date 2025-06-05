#include <SPI.h>
#include <LoRa.h>

uint8_t lastRands[10];
uint8_t randCount = 0;
uint8_t randIndex = 0;

// Czas ostatniego pomyślnego odbioru pakietu (w ms od uruchomienia)
unsigned long lastReceiveTime = 0;

// Funkcja inicjalizująca LoRa
void initLoRa() {
  Serial.begin(9600);
  while (!Serial);
  if (!LoRa.begin(868E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  Serial.println("=== LoRa ready ===");
}

// Sprawdza, czy Rand (ID pakietu) już wystąpił w ostatnich 10-ciu
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

// Modyfikuje pole o indeksie fieldIndex w pakiecie i odsyła
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

// Funkcja wysyłająca wiadomość „ZYJE” jako heartbeat
void sendAlive() {
  //Serial.println(" W1 is alive");
  LoRa.beginPacket();
    // Możesz tu dowolnie zmienić treść. Używamy metody print, więc ciąg znaków.
    LoRa.print("W1 is alive");
  LoRa.endPacket();
}

// Odbiór pakietu o długości 7 bajtów, dekodowanie i ewentualne odesłanie
void receiveMessage() {
  int packetSize = LoRa.parsePacket();
  if (packetSize <= 0) return;

  if (packetSize != 7) {
    // Jeśli długość nie jest równa 7, wyrzucamy bajty i wychodzimy
    while (LoRa.available()) {
      LoRa.read();
    }
    return;
  }

  // Mamy dokładnie 7 bajtów
  uint8_t packet[7];
  LoRa.readBytes(packet, 7);

  // Debug: wypisz bajty w formacie [XX]
  for (uint8_t i = 0; i < 7; i++) {
    Serial.print("[");
    if (packet[i] < 0x10) Serial.print('0');
    Serial.print(packet[i], HEX);
    Serial.print("]");
  }
  Serial.println();

  // Dekodowanie pól
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

  // Odnotowujemy, że coś przyszło – resetujemy timer
  lastReceiveTime = millis();

  bool duplicate = processRand(IDpaket);

  if (!duplicate) {
    // Jeśli nie duplikat, modyfikujemy pole W1 i odsyłamy
    modifyAndResend(packet, 1, 0b0001);
    Serial.println("Resent modified packet (W1 set to 0b0001).");
  }
}

void setup() {
  initLoRa();
  randomSeed(analogRead(A0));
  // Na starcie ustawiamy lastReceiveTime na teraz,
  // żeby po włączeniu nie wysyłać od razu ŻYJĘ
  lastReceiveTime = millis();
}

void loop() {
  // Najpierw sprawdzamy, czy przyszła jakaś wiadomość
  receiveMessage();

  // Sprawdzamy, czy minęło ponad 15 sekund od ostatniego odbioru
  if (millis() - lastReceiveTime > 15000UL) {
    // Jeśli tak, wysyłamy wiadomość ŻYJĘ i resetujemy timer
    sendAlive();
    lastReceiveTime = millis();
  }

  // Krótka pauza, żeby nie zatykać CPU
  delay(10);
}
