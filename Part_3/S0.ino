#include <SPI.h>
#include <LoRa.h>



void LedOn() {
        digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
}

void LedOff() {
        digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
}


void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    LedOn();
    delay(5000); //turns on the LED for 5 seconds if LoRa start fails
    LedOff();
    while (1);
  }
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
 

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
     Serial.print("' with RSSI ");
     Serial.println(LoRa.packetRssi());
  }
}
