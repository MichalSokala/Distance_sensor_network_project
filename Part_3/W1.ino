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
    Serial.println("LoRa repeater");

    if (!LoRa.begin(868E6)) {
        Serial.println("Starting LoRa failed!");
        LedOn();
        delay(5000); //turns on the LED for 5 seconds if LoRa start fails
        LedOff();
        while (1);
    }
    LoRa.setTxPower(14);
}

void loop() {
    Serial.print("Sending packet: ");
    Serial.println(distance);

    // send packet
    LoRa.beginPacket();
    LoRa.endPacket();

    delay(5000);
}