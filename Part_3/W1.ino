#include <SPI.h>
#include <LoRa.h>

#define S0_ID          0b0000
#define S1_ID          0b0001
#define W1_ID          0b1001

void LedOn() {
        digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
}

void LedOff() {
        digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
}

char mess;


void BlinkLED(int count) {
    for (int i = 0; i < count; i++) {
        digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
        delay(300);                      // Wait 300 ms
        digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
        delay(300);                      // Wait 300 ms before the next blink
    }
}


void setup() {
    Serial.begin(9600);
    Serial.println("LoRa repeater");

    if (!LoRa.begin(868E6)) {
        Serial.println("Starting LoRa failed!");
        LedOn();
        delay(5000); //turns on the LED for 5 seconds if LoRa start fails
        LedOff();
        while (1);
    }
    BlinkLED(4);
    LoRa.setTxPower(14);
}




void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");


    // read packet
    while (LoRa.available()) {
      mess = (char)LoRa.read();
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    LoRa.beginPacket();
    LoRa.print("siema");
    LoRa.print(mess);
    LoRa.endPacket();

    delay(5000);
  }
}