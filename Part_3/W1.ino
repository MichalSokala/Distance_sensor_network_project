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
    String mess = LoRa.receive();
    Serial.print("Received packet: ");
    Serial.println(mess);
    if (mess){
      String Fullmess = "The message repeated by " + String(W1_ID) + " is: " + mess;
      LoRa.beginPacket();
      LoRa.print(Fullmess);
      LoRa.endPacket();
    }

    // send packet
    LoRa.beginPacket();
    LoRa.endPacket();

    delay(5000);
}